//
// Created by kuba on 30.9.17.
//

#include <cassert>
#include <algorithm>
#include <cmath>

#include "commands_system.hpp"
#include "bytewriter.hpp"
#include "as_int.hpp"
#include "bytereader.hpp"

#define DOWNLOAD_CONTINUE_OVERHEAD 7
#define CONTINUE_GETFILE_OVERHEAD  12
#define MAX_DL_PAYLOAD (FILE_BUF_SIZE - DOWNLOAD_CONTINUE_OVERHEAD)
#define MAX_UP_PAYLOAD (FILE_BUF_SIZE - CONTINUE_GETFILE_OVERHEAD)

using namespace ev3loader;
using namespace ev3loader::commands;

// Simple constructors

download_command::download_command(packet_layer &conn, input_type const &rq)
        : data_command(conn, rq), m_window{m_dataIn.contents},
          m_done(0), m_steps(1 + (m_dataIn.contents.size() + MAX_DL_PAYLOAD - 1) / MAX_DL_PAYLOAD) {}

download_command::download_command(packet_layer &conn, const request_download &rq, progresscallback &&info)
        : data_command(conn, rq), m_window{m_dataIn.contents},
          m_done(0), m_steps(1 + (m_dataIn.contents.size() + MAX_DL_PAYLOAD - 1) / MAX_DL_PAYLOAD),
          simple_progress_reporter(std::move(info)) {}

upload_command::upload_command(packet_layer &conn, input_type const &rq)
        : data_command(conn, rq), simple_progress_reporter(), m_done(0), m_steps(0) {
}

upload_command::upload_command(packet_layer &conn, input_type const &rq, progresscallback &&info)
        : data_command(conn, rq), simple_progress_reporter(std::move(info)), m_done(0), m_steps(0) {
}

///////////////////////
// COMPOUND COMMANDS //
///////////////////////

// File Download

void ev3loader::commands::download_command::send_impl() {
    request_download_begin rq;
    rq.cmd          = m_state = DOWNLOAD_BEGIN;
    rq.file_name    = m_dataIn.path;
    rq.file_length  = m_dataIn.contents.size();
    rq.file_payload = buffer_view<>{};

    send_subcommand<download_begin>(&download_command::on_receive, rq);
}

void download_command::on_receive(reply_download rpl) {
    report_progress(++m_done, m_steps);
    m_dataOut = rpl;
    switch (rpl.result) {
        case SUCCESS:
            if (m_window.empty()) {
                throw parse_failed{LATE_EOF};
            } else {
                send_next();
            }
            return;
        case END_OF_FILE:
            if (m_window.empty()) {
                m_dataOut.result = SUCCESS;
                async_success();
            } else {
                throw parse_failed{EARLY_EOF};
            }
            return;
        default:
            throw sys_remote_error(m_state, rpl.result);
    }
}

void download_command::send_next() {
    buffer_view<> sub = m_window.subview(MAX_DL_PAYLOAD);

    request_download_continue rq;
    rq.cmd          = m_state = DOWNLOAD_CONTINUE;
    rq.file_id      = static_cast<file_handle>(m_dataOut.file_id);
    rq.file_payload = sub;

    send_subcommand<download_continue>(&download_command::on_receive, rq);
    m_window.consume(sub);
}

// Whatever upload

static system_commands get_upbegin_cmd(upload_type what) {
    switch (what) {
        case upload_type::regular:
            return UPLOAD_BEGIN;
        case upload_type::datalog:
            return GETFILE_BEGIN;
        case upload_type::ls:
            return LIST_FILES_BEGIN;
    }
}

static system_commands get_upcontinue_cmd(upload_type what) {
    switch (what) {
        case upload_type::regular:
            return UPLOAD_CONTINUE;
        case upload_type::datalog:
            return GETFILE_CONTINUE;
        case upload_type::ls:
            return LIST_FILES_CONTINUE;
    }
}

void upload_command::send_impl() {
    request_upload_begin rq;
    rq.cmd            = m_state = get_upbegin_cmd(m_dataIn.op);
    rq.file_name      = m_dataIn.path;
    rq.request_length = MAX_UP_PAYLOAD;

    send_subcommand<upload_begin>(&upload_command::receive_first, rq);
}

void upload_command::send_next() {
    request_upload_continue rq;
    rq.cmd            = m_state = get_upcontinue_cmd(m_dataIn.op);
    rq.file_id        = static_cast<file_handle>(m_dataOut.file_id);
    rq.file_length    = m_dataOut.file_length;
    rq.request_length = MAX_UP_PAYLOAD; // buffer crop is handled on the brick

    send_subcommand<upload_continue>(&upload_command::receive_later, rq);
}

void upload_command::receive_first(reply_upload rpl) {
    switch (rpl.result) {
        case SUCCESS: {
            m_dataOut.file_length = rpl.file_length;
            m_dataOut.file_id     = rpl.file_id;
            m_dataOut.result      = rpl.result;
            m_dataOut.file_payload.assign(m_dataOut.file_length, 0);
            m_window = buffer_view<>{m_dataOut.file_payload};

            buffer_view<> rxd{rpl.file_payload};
            m_window.copy_in(rxd);

            if (m_window.empty()) {
                throw parse_failed{LATE_EOF};
            } else {
                progress_update();
                send_next();
            }
            return;
        }
        case END_OF_FILE:
            m_dataOut = rpl;
            m_dataOut.result = SUCCESS;
            async_success();
            return;
        default:
            m_dataOut = rpl;
            throw sys_remote_error(m_state, rpl.result);
    }
}

void upload_command::receive_later(reply_upload rpl) {
    switch (rpl.result) {
        case SUCCESS: {
            refresh_length(rpl);
            m_dataOut.file_id = rpl.file_id;
            m_dataOut.result  = rpl.result;

            buffer_view<> rxd = buffer_view<>{rpl.file_payload};
            m_window.copy_in(rxd);

            if (m_window.empty()) {
                async_success();
            } else {
                progress_update();
                send_next();
            }
            return;
        }
        case END_OF_FILE:
            m_dataOut.result = SUCCESS;
            async_success();
            return;
        default:
            m_dataOut = rpl;
            throw sys_remote_error{m_state, m_dataOut.result};
    }
}

void upload_command::refresh_length(reply_upload &rpl) {
    if (m_dataOut.file_length != rpl.file_length) {
        m_dataOut.file_length = rpl.file_length;
        size_t diff = m_window.iter - m_window.begin;

        m_dataOut.file_payload.resize(rpl.file_length, 0);
        m_window.begin = m_dataOut.file_payload.begin();
        m_window.iter  = m_window.begin + diff;
        m_window.end   = m_dataOut.file_payload.end();
    }
}

void upload_command::progress_update() {
    m_steps = (m_dataOut.file_length + MAX_UP_PAYLOAD - 1) / MAX_UP_PAYLOAD;
    m_done++;
    report_progress(m_done, m_steps);
}


/////////////////////
// SIMPLE COMMANDS //
/////////////////////

// file operation (mkdir/unlink)

static system_commands get_fileop_cmd(fileop_type what) {
    switch (what) {
        case fileop_type::mkdir:
            return MKDIR;
        case fileop_type::unlink:
            return DELETE_FILE;
    }
}

void fileop_command::send_impl() {
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, get_fileop_cmd(m_dataIn.what))
        << as_string(m_dataIn.path);

    assert_buffer_single_ok(out.size());

    m_tx.buffer_send(out.buffer(), &fileop_command::on_receive);
}

void fileop_command::on_receive(const bytearray &pkt) {
    bytereader in(pkt);

    auto hdr = in.take<reply_system_header>();
    m_dataOut.result = hdr.result();
    in.finalize();

    assert_system_reply_ok(hdr, get_fileop_cmd(m_dataIn.what));
    async_success();
}

///////////////////////
// FILE I/O COMMANDS //
///////////////////////

// Begin Download

void download_begin::send_impl() {
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, m_dataIn.cmd);
    out << as_uint32(m_dataIn.file_length);
    out << as_string(m_dataIn.file_name);
    assert_buffer_single_ok(out.size());
    out << as_bytes(m_dataIn.file_payload);
    assert_buffer_large_ok(out.size());

    m_tx.buffer_send(out.buffer(), &download_begin::on_receive);
}


void download_begin::on_receive(const bytearray &pkt) {
    bytereader in(pkt);

    auto hdr = in.take<reply_system_header>();
    m_dataOut.result  = hdr.result();
    m_dataOut.file_id = in.take_handle();
    in.finalize();

    assert_system_reply_ok(hdr, m_dataIn.cmd);
    async_success();
}

// Continue Download

void download_continue::send_impl() {
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, m_dataIn.cmd)
        << as_handle(m_dataIn.file_id)
        << as_bytes(m_dataIn.file_payload);

    assert_buffer_large_ok(out.size());
    m_tx.buffer_send(out.buffer(), &download_continue::on_receive);
}

void download_continue::on_receive(const bytearray &pkt) {
    bytereader in(pkt);

    auto hdr = in.take<reply_system_header>();
    m_dataOut.result  = hdr.result();
    m_dataOut.file_id = in.take_handle();
    in.finalize();

    assert_system_reply_ok(hdr, m_dataIn.cmd);
    async_success();
}

// Begin Upload/GetFile/ListFiles

void upload_begin::send_impl() {
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, m_dataIn.cmd)
        << as_uint16(m_dataIn.request_length)
        << as_string(m_dataIn.file_name);

    assert_buffer_single_ok(out.size());

    m_tx.buffer_send(out.buffer(), &upload_begin::on_receive);
}

void upload_begin::on_receive(const bytearray &pkt) {
    bytereader in(pkt);

    auto hdr = in.take<reply_system_header>();
    m_dataOut.result       = hdr.result();
    m_dataOut.file_length  = in.take_uint32();
    m_dataOut.file_id      = in.take_handle();
    m_dataOut.file_payload = in.take_buffer();
    in.finalize();

    assert_system_reply_ok(hdr, m_dataIn.cmd);
    async_success();
}

// Continue Download/GetFile/ListFiles

void upload_continue::send_impl() {
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, m_dataIn.cmd)
        << as_handle(m_dataIn.file_id)
        << as_uint16(m_dataIn.request_length);

    m_tx.buffer_send(out.buffer(), &upload_continue::on_receive);
}

void upload_continue::on_receive(const bytearray &pkt) {
    bytereader in(pkt);

    auto hdr               = in.take<reply_system_header>();
    if (m_dataIn.cmd == GETFILE_CONTINUE) {
        m_dataOut.file_length = in.take_uint32();
    } else {
        m_dataOut.file_length = m_dataIn.file_length;
    }
    m_dataOut.file_id      = in.take_handle();
    m_dataOut.file_payload = in.take_buffer();
    in.finalize();

    assert_system_reply_ok(hdr, m_dataIn.cmd);
    async_success();
}

// List command + parsing

void ls_command::send_impl() {
    request_upload rq;
    rq.path = m_dataIn.path;
    rq.op   = upload_type::ls;

    send_subcommand<upload_command>(&ls_command::on_receive, rq);
}

uint8_t hex_letter(char letter) {
    if ('0' <= letter && letter <= '9') {
        return static_cast<uint8_t>(letter - '0');
    } else if ('a' <= letter && letter <= 'f') {
        return static_cast<uint8_t>(letter - 'a' + 10);
    } else if ('A' <= letter && letter <= 'F') {
        return static_cast<uint8_t>(letter - 'A' + 10);
    } else {
        throw std::invalid_argument("Hex string has invalid characters");
    }
}

void md5_parse(std::string const &in, std::array<uint8_t, 16> &out) {
    auto iIt = in.begin(), iEnd = in.end();
    auto oIt = out.begin(), oEnd = out.end();

    while (iIt != iEnd && oIt != oEnd) {
        uint8_t hi = hex_letter(*iIt++);
        uint8_t lo = hex_letter(*iIt++);

        *oIt++ = (uint8_t) (hi << 4 | lo);
    }
    if (iIt != iEnd || oIt != oEnd)
        throw std::out_of_range("Hash in/out bounds do not match");
}

std::vector<std::string> buffer_to_lines(bytearray const &in) {
    std::vector<std::string> out;
    auto                     pos = in.begin();
    auto                     end = in.end();

    while (pos < end) {
        auto linebegin = pos;
        auto lineend   = std::find(linebegin, end, '\n');
        out.emplace_back(linebegin, lineend); // exclude newline
        pos = lineend + 1;
    }
    return out;
}

void parse_dir(std::string const &line, dir_entry &entry) {
    entry.what = file_type::directory;
    entry.size = 0;
    entry.md5.fill(0);
    entry.name = line.substr(0, line.length() - 1);
}

void parse_file(std::string const &line, dir_entry &entry) {
    std::string str_md5  = line.substr(0, 32);
    std::string str_size = line.substr(33, 8);
    std::string str_name = line.substr(42);

    auto parsed_size = static_cast<uint32_t>(std::stol(str_size, nullptr, 16));

    entry.what = file_type::file;
    entry.size = parsed_size;
    md5_parse(str_md5, entry.md5);
    entry.name = str_name;
}

reply_ls lines_to_result(std::vector<std::string> const &lines) {
    reply_ls  out{};
    dir_entry entry;
    try {
        for (std::string const &line : lines) {
            if (line.back() == '/') { // folder
                parse_dir(line, entry);
            } else { // file
                parse_file(line, entry);
            }
            out.listing.push_back(std::move(entry));
        }
        out.result = SUCCESS;
    } catch (std::out_of_range &ex) {
        throw parse_failed{LS_PARSE_FAILED};
    } catch (std::invalid_argument &ex) {
        throw parse_failed{LS_PARSE_FAILED};
    }
    return out;
}

void ls_command::on_receive(reply_upload pkt) {
    if (pkt.result != SUCCESS) {
        throw sys_remote_error(LIST_FILES_BEGIN, pkt.result);
    }

    auto lines = buffer_to_lines(pkt.file_payload);
    m_dataOut = lines_to_result(lines);
    async_success();
}
// firmware update

void fwupdate_command::send_impl() {
    bytewriter out;
    out << as_system_header(SYS_SEND_NOREPLY, ENTERFWUPDATE);

    m_tx.buffer_send(out.buffer());
    async_success(dummy{});
}

void closehandle_command::send_impl() {
    bytewriter out;
    out << as_system_header(SYS_SEND_REPLY, CLOSE_FILEHANDLE)
        << as_handle(m_dataIn.fd);

    m_tx.buffer_send(out.buffer(), &closehandle_command::on_receive);
}

void closehandle_command::on_receive(const bytearray &pkt) {
    bytereader in(pkt);

    auto hdr = in.take<reply_system_header>();
    m_dataOut.fd = in.take_handle();
    in.finalize();

    assert_command_success(hdr, CLOSE_FILEHANDLE);
    async_success();
}

void write_mailbox_command::send_impl() {
    bytewriter out;

    if (m_dataIn.name.size() > 254) {
        throw std::logic_error("Name is longer than 254 bytes.");
    }

    out << as_system_header(SYS_SEND_NOREPLY, WRITEMAILBOX)
        << as_uint8(m_dataIn.name.size() + 1)
        << as_string(m_dataIn.name)
        << as_uint16(m_dataIn.payload.size())
        << as_bytes(m_dataIn.payload);

    assert_buffer_single_ok(out.size());

    m_tx.buffer_send(out.buffer());
    async_success();
}

void bluetooth_pin_command::send_impl() {
    uint8_t  *mac = m_dataIn.clientMac;
    uint32_t pin  = m_dataIn.brickPin;

    std::string macStr = format("%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    macStr.resize(12, 0);
    std::string pinStr = formatn("%u", 7, pin);
    size_t      pinLen = pinStr.length() + 1;
    pinStr.resize(6, 0);

    bytewriter out;
    out << as_system_header(SYS_SEND_REPLY, BLUETOOTHPIN)
        << as_uint8(13)
        << as_string(macStr)
        << as_uint8(pinLen)
        << as_string(pinStr);

    assert_buffer_single_ok(out.size());
    m_tx.buffer_send(out.buffer(), &bluetooth_pin_command::on_receive);
}

void bluetooth_pin_command::on_receive(const bytearray &pkt) {
    bytereader in(pkt);

    auto hdr     = in.take<reply_system_header>();
    auto macSize = in.take_uint8();
    auto mac     = in.take_string_block(13);
    auto pinSize = in.take_uint8();
    auto pin     = in.take_string_block(std::min(pinSize, (uint8_t) 7));
    in.finalize();

    assert_command_success(hdr, BLUETOOTHPIN);
    parse_assert(macSize == 13, REPLY_SIZE_MISMATCH);

    m_dataOut.brickPin = static_cast<uint32_t>(std::stol(pin, nullptr, 10));

    int n = std::sscanf(mac.c_str(), "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
                        m_dataOut.brickMac + 0, m_dataOut.brickMac + 1, m_dataOut.brickMac + 2,
                        m_dataOut.brickMac + 3, m_dataOut.brickMac + 4, m_dataOut.brickMac + 5);
    if (n != 6) {
        throw communication_error(i18n::priv::_("Invalid MAC format received."), HERE);
    }

    async_success();
}
