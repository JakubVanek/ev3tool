#include <ev3loader/commands/firmware/reboot.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::firmware;

reply_simple reboot::run() {
    bytewriter out;
    out << as_system_header(SYS_SEND_REPLY, FW_STARTAPP);

    bytereader in = communicate(out);

    auto hdr = in.take<reply_system_header>().verify(FW_STARTAPP);
    in.close();

    return reply_simple{
            hdr.result()
    };
}
