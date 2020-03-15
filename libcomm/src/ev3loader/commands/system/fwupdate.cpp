#include <ev3loader/commands/system/fwupdate.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

void enter_fwupdate::run() {
    bytewriter out;
    out << as_system_header(SYS_SEND_NOREPLY, ENTERFWUPDATE);

    just_send(out);
}
