#include <ev3loader/commands/direct/execute.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::direct;

execute::execute(packet_layer &conn,
                 buffer_view<> bytecode,
                 uint16_t globals,
                 uint16_t locals,
                 timeout_point due_by)
        : command(conn, due_by),
          m_code(bytecode),
          m_globals(globals),
          m_locals(locals) {}

bytewriter ev3loader::commands::direct::direct_request(message_type msg,
                                                       buffer_view<> code,
                                                       uint16_t globals,
                                                       uint16_t locals) {
    if (locals > 0x3F)
        throw std::logic_error("Too many locals in a direct command!");
    if (globals > 0x3FF)
        throw std::logic_error("Too many globals in a direct command!");

    auto firstByte = static_cast<uint8_t>(globals);
    auto secondByte = static_cast<uint8_t>((globals >> 8u) & 0x03u);
    secondByte |= locals << 2u;

    bytewriter out;

    out << as_size(0) // to be filled later
        << as_id(0) // to be filled later
        << as_type(msg)
        << as_uint8(firstByte)
        << as_uint8(secondByte)
        << as_bytes(code)
        << close_big{};

    return std::move(out);
}

reply_direct execute::run() {
    reply_direct reply;
    bytewriter out = direct_request(DIR_SEND_REPLY, m_code, m_globals, m_locals);
    bytereader in = communicate(out);

    auto size = in.take_size();
    auto id = in.take_id();
    reply.result = in.take_type() == DIR_RECV_REPLY ?
                   direct_command_result::SUCCESS :
                   direct_command_result::FAILURE;
    reply.globals = in.take_buffer();
    in.close();

    return std::move(reply);
}

