#include <ev3loader/commands/direct/execute_without_reply.hpp>
#include <ev3loader/commands/direct/execute.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::direct;

execute_without_reply::execute_without_reply(packet_layer &conn,
                                             buffer_view<> bytecode,
                                             uint16_t globals,
                                             uint16_t locals)
        : command(conn), m_code(bytecode), m_globals(globals), m_locals(locals) {}

void execute_without_reply::run() {
    bytewriter out = direct_request(DIR_SEND_NOREPLY, m_code, m_globals, m_locals);
    just_send(out);
}
