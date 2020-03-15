#include <ev3loader/commands/system/upload_continue.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

upload_continue::upload_continue(packet_layer &conn,
                                 file_handle handle,
                                 size_t nextSize,
                                 size_t totalSize,
                                 upload_type type,
                                 timeout_point due_by)
        : command(conn, due_by),
          m_handle(handle),
          m_next_request(nextSize),
          m_total_size(totalSize),
          m_command() {
    switch (type) {
        case upload_type::regular:
            m_command = UPLOAD_CONTINUE;
            break;
        case upload_type::datalog:
            m_command = GETFILE_CONTINUE;
            break;
        case upload_type::ls:
            m_command = LIST_FILES_CONTINUE;
            break;
    }
}

reply_upload upload_continue::run() {
    reply_upload reply = {};
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, m_command)
        << as_handle(m_handle)
        << as_uint16(m_next_request)
        << close_small();

    bytereader in = command::communicate(out);

    auto hdr = in.take<reply_system_header>().verify(m_command);
    if (m_command == GETFILE_CONTINUE)
        reply.file_length = in.take_uint32();
    else
        reply.file_length = m_total_size;
    reply.result = hdr.result();
    reply.file_id = in.take_handle();
    reply.file_payload = in.take_buffer();
    in.close();

    return reply;
}
