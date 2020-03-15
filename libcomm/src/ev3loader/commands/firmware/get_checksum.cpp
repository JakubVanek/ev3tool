#include <ev3loader/commands/firmware/get_checksum.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::firmware;

get_checksum::get_checksum(packet_layer &conn, uint32_t address, uint32_t length, timeout_point due_by)
        : command(conn, due_by), m_address(address), m_length(length) {}

crc32_t get_checksum::run() {
    bytewriter out;
    out << as_system_header(SYS_SEND_REPLY, FW_GETCRC32)
        << as_uint32(m_address)
        << as_uint32(m_length)
        << close_small{};

    bytereader in = communicate(out);

    auto hdr = in.take<reply_system_header>().verify(FW_GETCRC32);
    auto crc32 = in.take_uint32();
    in.close();

    return crc32;
}
