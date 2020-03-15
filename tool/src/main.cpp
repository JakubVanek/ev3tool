//
// Created by kuba on 28.7.18.
//

#include <iostream>
#include <fstream>
#include <zlib.h>
#include "localization_tool.hpp"
#include "config.hpp"
#include <ev3loader/exceptions.hpp>
#include <ev3loader/comm/transport_hid_except.hpp>
#include <ev3loader/comm/base.hpp>
#include <ev3loader/comm/connection_ev3.hpp>
#include <ev3loader/comm/transport_hid.hpp>
#include <ev3loader/utils/buffer.hpp>
#include <ev3loader/commands/system/download.hpp>
#include <ev3loader/commands/system/upload.hpp>
#include <ev3loader/commands/system/fileop.hpp>
#include <ev3loader/commands/system/ls.hpp>
#include <iomanip>
#include <ev3loader/commands/system/write_mbox.hpp>
#include <ev3loader/commands/system/set_bt_pin.hpp>
#include <ev3loader/commands/system/closefd.hpp>
#include <ev3loader/commands/system/fwupdate.hpp>
#include <ev3loader/commands/firmware/reboot.hpp>
#include <ev3loader/commands/firmware/get_version.hpp>
#include <ev3loader/commands/firmware/erase.hpp>
#include <ev3loader/commands/firmware/download.hpp>
#include <ev3loader/commands/firmware/get_checksum.hpp>
#include <ev3loader/commands/direct/execute.hpp>

using namespace ev3loader;
using namespace ev3loader::comm;
using namespace ev3tool::i18n;
using std::vector;
using std::string;
using ev3loader::utils::starts_with;
using ev3loader::utils::buffer_view;

namespace sys = ev3loader::commands::system;
namespace fw = ev3loader::commands::firmware;
namespace direct = ev3loader::commands::direct;

vector<string> convert_args(int argc, char **argv) {
    vector<string> result;

    for (int i = 1; i < argc; i++) {
        result.emplace_back(argv[i]);
    }

    return result;
}

bytearray read_file(const std::string &path, std::istream &in) {
    bytearray result(std::istreambuf_iterator<char>{in},
                     std::istreambuf_iterator<char>{});
    // validate
    if (in.fail()) {
        throw std::runtime_error(_("Cannot read file"));
    } else {
        return result;
    }
}

bytearray read_file(const std::string &path) {
    if (path == "-") {
        return read_file(path, std::cin);
    } else {
        std::ifstream in(path, std::ios::binary | std::ios::in);
        return read_file(path, in);
    }
}

void write_file(const std::string &path, bytearray &data, std::ostream &out) {
    // write
    out.write(reinterpret_cast<const char *>(data.data()), data.size());
    // validate
    if (out.fail()) {
        throw std::runtime_error(_("Cannot write file"));
    }
}

void write_file(const std::string &path, bytearray &data) {
    if (path == "-") {
        return write_file(path, data, std::cout);
    } else {
        std::ofstream out(path, std::ios::out | std::ios::trunc | std::ios::binary);
        return write_file(path, data, out);
    }
}

struct command_params {
    command_params() = default;

    bool usb = false;
    bool tcp = false;
    bool serial = false;

    string ip_address;
    string serial_in;
    string serial_out;

    vector<string> positional;

    void require_sub_args(int count) {
        if (positional.size() != count + 1)
            throw std::runtime_error("Wrong number of parameters");
    }
};

uLong crc32_fw(bytearray &fw) {
    uLong crc = crc32(0L, Z_NULL, 0L);
    crc = crc32(crc, fw.data(), fw.size());
    return crc;
}

void hue_assert(bool current, bool request) {
    if (current != request) {
        if (current)
            throw std::runtime_error("Needs normal mode");
        else
            throw std::runtime_error("Needs FW mode");
    }
}

int do_run(int argc, char **argv) {
    string progName = argv[0];
    vector<string> options = convert_args(argc, argv);

    command_params cmd;

    for (const string &param : options) {
        if (param == "-h" || param == "--help" || param == "/?") {
            std::cout << "Help:" << std::endl;
            std::cout << "up        ... Upload a file to the brick." << std::endl;
            std::cout << "dl        ... Download a file from the brick." << std::endl;
            std::cout << "mkdir     ... Creates a directory on the brick." << std::endl;
            std::cout << "ls        ... Print directory listing from the brick." << std::endl;
            std::cout << "rm        ... Remove a file from the brick." << std::endl;
            std::cout << "mailbox   ... Write into EV3 mailbox." << std::endl;
            std::cout << "btpin     ... Set BT pin." << std::endl;
            std::cout << "listhnds  ... List all open handles." << std::endl;
            std::cout << "closehnds ... Try to close all handles." << std::endl;
            std::cout << "closehnd  ... Close only one handle." << std::endl;
            std::cout << "fwenter   ... Reboot into bootloader/firmware update mode." << std::endl;
            std::cout << "fwexit    ... Reboot out of bootloader/firmware update mode." << std::endl;
            std::cout << "fwver     ... Get HW/FW version from bootloader mode." << std::endl;
            std::cout << "fwflash   ... Flash a firmware file to the brick." << std::endl;
            return 0;

        } else if (param == "--version") {
            std::cout << TOOL_NAME " " TOOL_VERSION << std::endl;
            return 0;

        } else if (param == "--printexits") {
            std::cout << "Exits are cool" << std::endl;
            return 0;

        } else if (starts_with(param, 0, "--tcp")) {
            cmd.tcp = true;
            if (starts_with(param, 5, "="))
                cmd.ip_address = param.substr(6);

        } else if (starts_with(param, 0, "--usb")) {
            cmd.usb = true;

        } else if (starts_with(param, 0, "--serial")) {
            cmd.serial = true;
            if (starts_with(param, 8, "=")) {
                std::string raw_content = param.substr(9);

                auto offset = raw_content.find(',');
                if (offset != string::npos) {
                    cmd.serial_in = raw_content.substr(0, offset);
                    cmd.serial_out = raw_content.substr(offset + 1, 0);
                } else {
                    cmd.serial_in = raw_content;
                    cmd.serial_out = raw_content;
                }
            }

        } else if (!starts_with(param, 0, "--")) {
            cmd.positional.push_back(param);

        } else {
            throw std::runtime_error("unknown parameter: " + param);
        }
    }

    if (cmd.positional.empty())
        throw std::runtime_error("no command given");
    string command = cmd.positional[0];

    bool is_fw = false;
    std::unique_ptr<channel> chan;
    if (cmd.tcp) {
        throw std::runtime_error("tcp not yet supported");

    } else if (cmd.serial) {
        throw std::runtime_error("serial not yet supported");
    } else /* if (cmd.usb) */ {
        auto devices = hidapi_link::discover();

        if (!devices.empty()) {
            is_fw = devices[0].is_ev3_fwmode();
            chan = std::make_unique<hidapi_link>(devices[0]);
        } else {
            throw std::runtime_error("cannot find any brick");
        }
    }

    std::unique_ptr<packet_layer> ev3 = std::make_unique<ev3_connection>(std::move(chan));

    timeout_point due_long = std::chrono::steady_clock::now() + std::chrono::hours(1);

    if (command == "up") {
        cmd.require_sub_args(2);
        hue_assert(is_fw, false);

        sys::download{*ev3,
                      cmd.positional[2],
                      read_file(cmd.positional[1]),
                      due_long}.sync();

        bytearray beep_code = {0x82, 0x0f, 0x0a};
        auto view = buffer_view<>{beep_code};

        direct::execute{*ev3, view, 0, 0, due_long}.sync();

    } else if (command == "down") {
        cmd.require_sub_args(2);
        hue_assert(is_fw, false);

        auto reply = sys::upload{*ev3,
                                 cmd.positional[1],
                                 sys::upload_type::datalog,
                                 due_long}.sync();

        write_file(cmd.positional[2], reply.file_payload);

    } else if (command == "rm") {
        cmd.require_sub_args(1);
        hue_assert(is_fw, false);

        sys::fileop_command{*ev3, cmd.positional[1],
                            sys::fileop_type::unlink,
                            due_long}.sync();

    } else if (command == "ls") {
        cmd.require_sub_args(1);
        hue_assert(is_fw, false);

        auto out = sys::ls{*ev3, cmd.positional[1], due_long}.sync();

        for (auto &&elem : out.listing) {
            std::cout << (elem.what == sys::file_type::directory ? "dir " : "fil ");
            std::cout << std::setw(10) << std::setfill(' ') << std::dec << elem.size << std::setw(0);
            std::cout << " " << elem.name << (elem.what == sys::file_type::directory ? "/" : "");
            if (elem.what == sys::file_type::file) {
                std::cout << " [";
                std::cout << std::hex << std::setfill('0');
                for (uint8_t byte : elem.md5) {
                    std::cout << std::setw(2) << (int) byte;
                }
                std::cout << std::setw(0) << "]";
            }
            std::cout << std::endl;
        }

    } else if (command == "info") {
        throw std::runtime_error("info is not yet implemented");

    } else if (command == "mkdir") {
        cmd.require_sub_args(1);
        hue_assert(is_fw, false);

        sys::fileop_command{*ev3, cmd.positional[1],
                            sys::fileop_type::mkdir,
                            due_long}.sync();

    } else if (command == "mkrbf") {
        throw std::runtime_error("mkrbf is not yet implemented");

    } else if (command == "run") {
        throw std::runtime_error("run is not yet implemented");

    } else if (command == "exec") {
        throw std::runtime_error("exec is not yet implemented");

    } else if (command == "tunnel") {
        throw std::runtime_error("tunnel not yet implemented");

    } else if (command == "bridge") {
        throw std::runtime_error("bridge not yet implemented");

    } else if (command == "mailbox") {
        cmd.require_sub_args(2);
        hue_assert(is_fw, false);
        sys::write_mbox{*ev3,
                        cmd.positional[1],
                        read_file(cmd.positional[2]),
                        due_long}.sync();

    } else if (command == "btpin") {
        cmd.require_sub_args(2);
        hue_assert(is_fw, false);

        std::string target_pin = cmd.positional[1];
        std::string client_mac = cmd.positional[2];

        uint32_t brickPin;
        uint8_t clientMac[6];
        brickPin = (uint32_t) std::stoul(target_pin, nullptr, 10);
        int n = std::sscanf(client_mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                            clientMac + 0, clientMac + 1, clientMac + 2,
                            clientMac + 3, clientMac + 4, clientMac + 5);
        if (n != 6) {
            throw communication_error(_("Invalid format of the provided MAC address."), HERE);
        }

        std::cout << _("Sending pin change request...") << std::endl;
        auto result = sys::set_bt_pin(*ev3, clientMac, brickPin, due_long).sync();

        std::cout << _("Success! Brick blutooth info:") << std::endl;
        std::cout << format(_("- brick PIN: %u"), result.brickPin) << std::endl;
        std::cout << format(_("- brick MAC: %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX"),
                            result.brickMac[0], result.brickMac[1], result.brickMac[2],
                            result.brickMac[3], result.brickMac[4], result.brickMac[5]) << std::endl;

    } else if (command == "closehandles") {
        cmd.require_sub_args(0);

        for (int i = 0; i <= 0xFF; i++) {
            try {
                sys::closefd(*ev3, i, due_long).sync();
            } catch (...) {
            }
        }

    } else if (command == "closehandle") {
        cmd.require_sub_args(1);
        hue_assert(is_fw, false);

        long handle = std::stol(cmd.positional[1]);
        sys::closefd(*ev3, handle, due_long).sync();

    } else if (command == "fwenter") {
        cmd.require_sub_args(0);
        hue_assert(is_fw, false);

        sys::enter_fwupdate(*ev3).sync();

    } else if (command == "fwexit") {
        cmd.require_sub_args(0);
        hue_assert(is_fw, true);

        fw::reboot(*ev3, due_long).sync();

    } else if (command == "fwver") {
        cmd.require_sub_args(0);
        hue_assert(is_fw, true);

        auto ver = fw::get_version(*ev3, due_long).sync();

        char hwMajor = static_cast<char>('0' + (ver.hardwareVersion / 10) % 10);
        char hwMinor = static_cast<char>('0' + (ver.hardwareVersion / 1) % 10);
        char fwMajor = static_cast<char>('0' + (ver.firmwareVersion / 10) % 10);
        char fwMinor = static_cast<char>('0' + (ver.firmwareVersion / 1) % 10);

        std::cout << format(_("Hardware version: V%c.%c0"), hwMajor, hwMinor) << std::endl;
        std::cout << format(_("EEPROM   version: V%c.%c0"), fwMajor, fwMinor) << std::endl;

    } else if (command == "fwflash") {
        cmd.require_sub_args(1);
        hue_assert(is_fw, true);

        std::cout << "Loading FW..." << std::endl;
        auto fw = read_file(cmd.positional[1]);
        uLong crc = crc32_fw(fw);

        std::cout << "Erasing..." << std::endl;
        fw::erase(*ev3, due_long).sync();

        std::cout << "Flashing..." << std::endl;
        fw::download(*ev3, 0x00000000, fw, due_long).sync();

        std::cout << _("Verifying CRC...") << std::endl;

        auto crc32 = fw::get_checksum(*ev3, 0x00000000, fw.size(), due_long).sync();

        if (crc32 == crc) {
            std::cout << format(_("CRC32 matches: 0x%08x"), crc32) << std::endl;
        } else {
            std::cout << format(
                    _("CRC32 does not match (0x%08x remote, 0x%08x local)"),
                    crc32,
                    crc) << std::endl;
            throw communication_error(_("CRC32 mismatch"), HERE);
        }

        fw::reboot(*ev3, due_long).sync();
    } else {
        throw std::runtime_error("unknown command: " + command);
    }
    return 0;
}

int main(int argc, char **argv) {
    try {
        return do_run(argc, argv);
    } catch (hidapi_error &ex) {
        std::cerr << _("HID error: ") << ex.what() << std::endl;
        return 1;
    } catch (transmit_failed &ex) {
        std::cerr << _("Transmission error: ") << ex.what() << std::endl;
        return 2;
    } catch (abort_error &ex) {
        std::cerr << _("Operation aborted: ") << ex.what() << std::endl;
        return 3;
    } catch (timeout_error &ex) {
        std::cerr << _("Operation timed out: ") << ex.what() << std::endl;
        return 4;
    } catch (parse_failed &ex) {
        std::cerr << _("Message error: ") << ex.what() << std::endl;
        return 5;
    } catch (sys_remote_error &ex) {
        std::cerr << _("Remote error: ") << ex.what() << std::endl;
        return 6;
    } catch (communication_error &ex) {
        std::cerr << _("Communication error: ") << ex.what() << std::endl;
        return 7;
    } catch (std::exception &ex) {
        std::cerr << _("Generic error: ") << ex.what() << std::endl;
        return 8;
    } catch (...) {
        std::cerr << _("Unknown error") << std::endl;
        return 9;
    }
}


