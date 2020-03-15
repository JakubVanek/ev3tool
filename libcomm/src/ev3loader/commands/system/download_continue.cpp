#include <ev3loader/commands/system/download_continue.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

download_continue::download_continue(packet_layer &conn,
                                     file_handle handle,
                                     buffer_view<> segment,
                                     timeout_point due_by)
        : command(conn, due_by),
          m_handle(handle),
          m_segment(segment) {}

reply_download download_continue::run() {
    reply_download reply = {};
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, DOWNLOAD_CONTINUE)
        << as_handle(m_handle)
        << as_bytes(m_segment)
        << close_big{};

    bytereader in = command::communicate(out);

    auto hdr = in.take<reply_system_header>().verify(DOWNLOAD_CONTINUE);
    reply.result = hdr.result();
    reply.file_id = in.take_handle();
    in.close();

    return reply;
}
