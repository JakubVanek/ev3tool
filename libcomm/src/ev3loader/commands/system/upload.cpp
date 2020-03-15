#include <ev3loader/commands/system/upload.hpp>
#include <ev3loader/commands/system/upload_begin.hpp>
#include <ev3loader/commands/system/upload_continue.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

upload::upload(packet_layer &conn,
               std::string path,
               upload_type which,
               timeout_point due_by)
        : command(conn, due_by),
          m_path(std::move(path)),
          m_op(which),
          m_whole(),
          m_last_command(COMMAND_ERROR) {
    m_whole.file_length = 0;
    m_whole.file_payload.clear();
    m_window = buffer_view<>{m_whole.file_payload};
}

reply_upload upload::run() {
    bool goNext = process(send_first());

    while (goNext)
        goNext = process(send_next());

    m_whole.result = SUCCESS;
    return m_whole;
}


reply_upload upload::send_first() {
    upload_begin request(*pConn,
                         std::move(m_path),
                         MAX_UP_PAYLOAD,
                         m_op,
                         due_by);
    m_last_command = request.get_command();
    return request.sync();
}

reply_upload upload::send_next() {
    upload_continue request(*pConn,
                            m_whole.file_id,
                            MAX_UP_PAYLOAD, // buffer crop is handled on the brick
                            m_whole.file_length,
                            m_op,
                            due_by);
    m_last_command = request.get_command();
    return request.sync();
}

bool upload::process(reply_upload &&reply) {
    if (reply.result != SUCCESS && reply.result != END_OF_FILE)
        throw sys_remote_error(m_last_command, reply.result);

    m_whole.file_id = reply.file_id;
    m_whole.file_length = handle_new_size(reply);

    buffer_view<> rxd = buffer_view<>{reply.file_payload};
    m_window.copy_in(rxd);

    return decide_on_exitcode(reply.result);
}

size_t upload::handle_new_size(reply_upload &reply) {
    if (m_whole.file_length != reply.file_length) {
        m_whole.file_payload.resize(reply.file_length, 0);

        size_t done = m_window.consumed();
        m_window = buffer_view<>(m_whole.file_payload);
        m_window.skip(done);
    }
    return reply.file_length;
}

bool upload::decide_on_exitcode(system_status reply) {
    switch (reply) {
        case SUCCESS:
            if (m_window.empty())
                throw parse_failed{LATE_EOF};
            else
                return true;
        case END_OF_FILE:
            if (m_window.empty())
                return false;
            else
                throw parse_failed{EARLY_EOF};
        default:
            throw std::logic_error("should not be here");
    }
}
