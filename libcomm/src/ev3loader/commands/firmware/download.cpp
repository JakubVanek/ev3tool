#include <ev3loader/commands/firmware/download.hpp>
#include <ev3loader/commands/firmware/download_begin.hpp>
#include <ev3loader/commands/firmware/download_continue.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::firmware;

#define FWDL_OVERHEAD 6
#define MAX_FW_PAYLOAD (FILE_BUF_SIZE - FWDL_OVERHEAD)

download::download(packet_layer &conn, uint32_t address, bytearray data, timeout_point due_by)
        : command(conn, due_by),
          m_address(address),
          m_buffer(std::move(data)),
          m_window(m_buffer) {}

reply_simple download::run() {
    reply_simple reply = download_begin(*pConn, m_address, m_buffer.size(), due_by).sync();

    switch (reply.result) {
        case SUCCESS:
        case END_OF_FILE:
            if (m_window.empty()) {
                reply.result = SUCCESS;
                return reply;
            }
            else
                break;
        default:
            throw sys_remote_error(FW_DOWNLOAD_BEGIN, reply.result);
    }


    for (;;) {
        buffer_view<> sub = m_window.subview(MAX_FW_PAYLOAD);
        m_window.consume(sub);

        reply = download_continue(*pConn, sub, due_by).sync();
        switch (reply.result) {
            case SUCCESS:
            case END_OF_FILE:
                if (m_window.empty())
                    return reply;
                else
                    continue;
            default:
                throw sys_remote_error(FW_DOWNLOAD_CONTINUE, reply.result);
        }
    }
}
