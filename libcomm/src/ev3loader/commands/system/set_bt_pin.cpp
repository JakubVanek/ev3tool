#include <ev3loader/commands/system/set_bt_pin.hpp>


using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

set_bt_pin::set_bt_pin(packet_layer &conn,
                       uint8_t hostAddress[6],
                       uint32_t brickPin,
                       timeout_point due_by)
        : command(conn, due_by),
          m_host_address(),
          m_brick_pin(brickPin) {
    memcpy(m_host_address, hostAddress, sizeof(m_host_address));
}

reply_setpin set_bt_pin::run() {
    return process(send());
}

bytereader set_bt_pin::send() {
    uint8_t *mac = m_host_address;

    std::string macStr = format("%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    macStr.resize(12, 0);

    std::string pinStr = formatn("%u", 7, m_brick_pin);
    pinStr.resize(6, 0);

    bytewriter out;
    out << as_system_header(SYS_SEND_REPLY, BLUETOOTHPIN)
        << as_u8len_null_terminated_string(macStr)
        << as_u8len_null_terminated_string(pinStr)
        << close_small{};

    return communicate(out);
}

reply_setpin set_bt_pin::process(bytereader &&in) {
    reply_setpin output = {};

    auto hdr = in.take<reply_system_header>().verify(BLUETOOTHPIN);
    auto macSize = in.take_uint8();
    auto mac = in.take_string_block(macSize);
    auto pinSize = in.take_uint8();
    auto pin = in.take_string_block(pinSize);
    in.close();

    output.brickPin = static_cast<uint32_t>(std::stol(pin, nullptr, 10));

    int n = std::sscanf(mac.c_str(), "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
                        output.brickMac + 0, output.brickMac + 1, output.brickMac + 2,
                        output.brickMac + 3, output.brickMac + 4, output.brickMac + 5);
    if (n != 6) {
        throw communication_error(i18n::priv::_("Invalid MAC format received."), HERE);
    }

    return output;
}
