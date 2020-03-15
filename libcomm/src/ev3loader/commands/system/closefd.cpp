#include <ev3loader/commands/system/closefd.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

closefd::closefd(packet_layer &conn, file_handle handle, timeout_point due_by)
        : command(conn, due_by),
          m_handle(handle) {}

reply_simple closefd::run() {
    reply_simple reply = {};
    bytewriter out;
    out << as_system_header(SYS_SEND_REPLY, CLOSE_FILEHANDLE)
        << as_handle(m_handle)
        << close_small{};

    bytereader in = communicate(out);

    auto hdr = in.take<reply_system_header>().verify(CLOSE_FILEHANDLE);
    reply.result = hdr.result();
    (void) in.take_handle();
    in.close();

    return reply;
}
