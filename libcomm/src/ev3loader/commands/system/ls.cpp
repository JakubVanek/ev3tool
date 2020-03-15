#include <ev3loader/commands/system/ls.hpp>
#include <ev3loader/commands/system/upload.hpp>
#include <algorithm>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

static void parse_dir(const std::string &line, dir_entry &entry);

static void parse_file(const std::string &line, dir_entry &entry);

static std::vector<std::string> buffer_to_lines(bytearray const &in);

static reply_ls lines_to_result(std::vector<std::string> const &lines);


ls::ls(packet_layer &conn, std::string path, timeout_point due_by)
        : command(conn, due_by), m_path(std::move(path)) {}

reply_ls ls::run() {
    auto reply = upload(*pConn, m_path, upload_type::ls, due_by).sync();

    if (reply.result != SUCCESS) {
        throw sys_remote_error(LIST_FILES_BEGIN, reply.result);
    }

    auto lines = buffer_to_lines(reply.file_payload);
    return lines_to_result(lines);
}

reply_ls lines_to_result(std::vector<std::string> const &lines) {
    reply_ls out{};
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

        *oIt++ = (uint8_t) (hi << 4u | lo);
    }
    if (iIt != iEnd || oIt != oEnd)
        throw std::out_of_range("Hash in/out bounds do not match");
}

std::vector<std::string> buffer_to_lines(bytearray const &in) {
    std::vector<std::string> out;
    auto pos = in.begin();
    auto end = in.end();

    while (pos < end) {
        auto linebegin = pos;
        auto lineend = std::find(linebegin, end, '\n');
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
    std::string str_md5 = line.substr(0, 32);
    std::string str_size = line.substr(33, 8);
    std::string str_name = line.substr(42);

    auto parsed_size = static_cast<uint32_t>(std::stol(str_size, nullptr, 16));

    entry.what = file_type::file;
    entry.size = parsed_size;
    md5_parse(str_md5, entry.md5);
    entry.name = str_name;
}