#include <ev3loader/commands/system/write_mbox.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

write_mbox::write_mbox(packet_layer &conn,
                       std::string name,
                       bytearray data,
                       timeout_point due_by)
        : command(conn, due_by),
          m_name(std::move(name)),
          m_data(std::move(data)) {}

void write_mbox::run() {
    bytewriter out;

    out << as_system_header(SYS_SEND_NOREPLY, WRITEMAILBOX)
        << as_u8len_null_terminated_string(m_name)
        << as_u16len_bytes(m_data)
        << close_small{};

    just_send(out);
}
