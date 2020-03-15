#include <ev3loader/commands/firmware/download_continue.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::firmware;

download_continue::download_continue(packet_layer &conn, buffer_view<> data, timeout_point due_by)
        : command(conn, due_by), m_data(data) {}

reply_simple download_continue::run() {
    reply_simple result = {};
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, FW_DOWNLOAD_CONTINUE)
        << as_bytes(m_data)
        << close_small{};

    bytereader in = communicate(out);

    auto hdr = in.take<reply_system_header>().verify(FW_DOWNLOAD_CONTINUE);
    result.result = hdr.result();
    in.close();

    return result;
}
