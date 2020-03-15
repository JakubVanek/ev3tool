#include <ev3loader/commands/system/fileop.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

fileop_command::fileop_command(packet_layer &conn,
                               std::string path,
                               fileop_type what,
                               timeout_point due_by)
        : command(conn, due_by),
          m_path(std::move(path)),
          m_command() {

    switch (what) {
    case fileop_type::mkdir:
        m_command = MKDIR;
        break;
    case fileop_type::unlink:
        m_command = DELETE_FILE;
        break;
    }
}

reply_simple fileop_command::run() {
    reply_simple reply = {};
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, m_command)
        << as_string(m_path)
        << close_small{};

    bytereader in = communicate(out);

    auto hdr = in.take<reply_system_header>().verify(m_command);
    reply.result = hdr.result();
    in.close();

    return reply;
}
