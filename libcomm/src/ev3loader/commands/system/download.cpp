#include <ev3loader/commands/system/download.hpp>
#include <ev3loader/commands/system/download_begin.hpp>
#include <ev3loader/commands/system/download_continue.hpp>
#include <ev3loader/commands/firmware/download.hpp>

using namespace ev3loader;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

download::download(packet_layer &conn,
                   std::string path,
                   bytearray data,
                   timeout_point due_by)
        : command(conn, due_by),
          m_path(std::move(path)),
          m_data(std::move(data)),
          m_window(m_data),
          m_handle(0) {}

reply_download download::run() {
    reply_download current;
    for (size_t i = 0;; i++) {
        current = i == 0 ? send_first()
                         : send_next();

        if (!process(current, i == 0)) {
            return m_result;
        }
    }
}

reply_download download::send_first() {
    return download_begin(*pConn,
                          m_path,
                          m_window.available(),
                          due_by).sync();
}

reply_download download::send_next() {
    buffer_view<> sub = m_window.subview(MAX_DL_PAYLOAD);
    m_window.consume(sub);

    return download_continue(*pConn, m_handle, sub, due_by).sync();
}

bool download::process(reply_download &reply, bool first) {
    switch (reply.result) {
    case SUCCESS:
        if (m_window.empty())
            throw parse_failed{LATE_EOF};
        else
            return true;
    case END_OF_FILE:
        if (m_window.empty()) {
            m_result = reply;
            m_result.result = SUCCESS;
            return false;
        } else
            throw parse_failed{EARLY_EOF};
    default:
        throw sys_remote_error(first
                               ? DOWNLOAD_BEGIN
                               : DOWNLOAD_CONTINUE,
                               reply.result);
    }
}
