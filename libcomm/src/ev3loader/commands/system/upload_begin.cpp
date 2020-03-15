#include <ev3loader/commands/system/upload_begin.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

upload_begin::upload_begin(packet_layer &conn,
                           std::string path,
                           size_t nextSize,
                           upload_type type,
                           timeout_point due_by)
        : command(conn, due_by),
          m_path(std::move(path)),
          m_next_request(nextSize),
          m_command() {
    switch (type) {
        case upload_type::regular:
            m_command = UPLOAD_BEGIN;
            break;
        case upload_type::datalog:
            m_command = GETFILE_BEGIN;
            break;
        case upload_type::ls:
            m_command = LIST_FILES_BEGIN;
            break;
    }
}

reply_upload upload_begin::run() {
    reply_upload reply = {};
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, m_command)
        << as_uint16(m_next_request)
        << as_string(m_path)
        << close_small{};

    bytereader in = command::communicate(out);

    auto hdr = in.take<reply_system_header>().verify(m_command);
    reply.result = hdr.result();
    reply.file_length = in.take_uint32();
    reply.file_id = in.take_handle();
    reply.file_payload = in.take_buffer();
    in.close();

    return reply;
}
