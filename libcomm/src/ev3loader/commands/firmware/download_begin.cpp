#include <ev3loader/commands/firmware/download_begin.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::firmware;

download_begin::download_begin(packet_layer &conn, uint32_t address, uint32_t length, timeout_point due_by)
        : command(conn, due_by), m_address(address), m_length(length) {}

reply_simple download_begin::run() {
    reply_simple result = {};
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, FW_DOWNLOAD_BEGIN)
        << as_uint32(m_address)
        << as_uint32(m_length)
        << close_small{};

    bytereader in = communicate(out);

    auto hdr = in.take<reply_system_header>().verify(FW_DOWNLOAD_BEGIN);
    result.result = hdr.result();
    in.close();

    return result;
}
