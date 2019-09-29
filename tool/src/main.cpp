//
// Created by kuba on 28.7.18.
//

#include <iomanip>
#include <fstream>

#include <libintl.h>

#include <localization.hpp>
#include <connection_ev3.hpp>
#include <connection_ev3_hid.hpp>
#include <commands_system.hpp>
#include <commands_firmware.hpp>

#include "config.hpp"
#include "localization_tool.hpp"
#include "cmdline_parser.hpp"
#include "actions.hpp"

using namespace ev3loader;
using namespace ev3loader::i18n;
using namespace ev3loader::commands;
using namespace ev3tool;
using namespace ev3tool::program;
using namespace ev3tool::i18n;

int main(int argc, char **argv) {
    context                 ctx;
    ev3tool_bootstrap       start;
    std::unique_ptr<action> action;
    try {
        start.process(argc, argv);
        action = start.take_action();
    } catch (std::exception &ex) {
        std::cerr << _("Error parsing arguments: ") << ex.what() << std::endl << std::endl;
    }
    ctx.verbosity = start.get_verbosity();

    try {
        if (action != nullptr) {
            action->perform(ctx);
        } else {
            std::cout << _("Nothing to do.") << std::endl;
        }
    } catch (hidapi_error &ex) {
        std::cerr << _("HID error: ") << ex.what() << std::endl;
    } catch (transmit_failed &ex) {
        std::cerr << _("Transmission error: ") << ex.what() << std::endl;
    } catch (abort_error &ex) {
        std::cerr << _("Operation aborted: ") << ex.what() << std::endl;
    } catch (timeout_error &ex) {
        std::cerr << _("Operation timed out: ") << ex.what() << std::endl;
    } catch (parse_failed &ex) {
        std::cerr << _("Message error: ") << ex.what() << std::endl;
    } catch (sys_remote_error &ex) {
        std::cerr << _("Remote error: ") << ex.what() << std::endl;
    } catch (communication_error &ex) {
        std::cerr << _("Communication error: ") << ex.what() << std::endl;
    } catch (std::exception &ex) {
        std::cerr << _("Generic error: ") << ex.what() << std::endl;
    } catch (...) {
        std::cerr << _("Unknown error") << std::endl;
    }
}


