#include <ev3loader/commands/firmware/get_version.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::firmware;

version get_version::run() {
    version result = {};
    bytewriter out;

    out << as_system_header(SYS_SEND_REPLY, FW_GETVERSION);

    bytereader in = communicate(out);

    auto hdr = in.take<reply_system_header>().verify(FW_GETVERSION);
    result.hardwareVersion = in.take_uint32();
    result.firmwareVersion = in.take_uint32();
    in.close();

    return result;
}
