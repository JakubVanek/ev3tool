//
// Created by kuba on 11.8.18.
//

#include "actions.hpp"
#include "localization_tool.hpp"
#include "cmdline_parser.hpp"
#include "config.hpp"

#include <connection_ev3_hid.hpp>
#include <connection_ev3.hpp>
#include <commands.hpp>
#include <commands_system.hpp>
#include <commands_direct.hpp>
#include <commands_firmware.hpp>

#include <fstream>
#include <iomanip>
#include <utility>
#include <zlib.h>

using namespace ev3loader;
using namespace ev3tool::i18n;

namespace ev3tool {
    namespace program {

        int percent_old = 0;

        static void progress_report(size_t done, size_t total) {
            int percent = static_cast<int>(100 * done / total);
            if (percent != percent_old) {
                percent_old = percent;
                std::cout << "\r" << format(_("Progress: %3d %%"), percent);
                std::cout.flush();
            }
        }

        upload_action::upload_action(const std::string &from, const std::string &to)
                : m_from(from), m_to(to) {}

        download_action::download_action(const std::string &from, const std::string &to)
                : m_from(from), m_to(to) {}

        ls_action::ls_action(const std::string &from, bool md5)
                : m_dir(from), m_md5(md5) {}

        rm_action::rm_action(const std::string &from)
                : m_path(from) {}

        mkdir_action::mkdir_action(const std::string &to)
                : m_path(to) {}

        closehandle_action::closehandle_action() = default;

        listhandle_action::listhandle_action() = default;

        closehandle_single_action::closehandle_single_action(file_handle hnd) : m_handle(hnd) {}

        version_action::version_action() = default;

        fwversion_action::fwversion_action() = default;

        fwexit_action::fwexit_action() = default;

        fwenter_action::fwenter_action() = default;

        fwdl_action::fwdl_action(const std::string &from) : m_from(from) {}

        hid_connect::hid_connect(ev3loader::hid_address addr) : m_address(std::move(addr)) {}

        hid_list::hid_list() = default;
        mailbox_action::mailbox_action(const std::string &src_file, const std::string &dst_mbox)
                : m_source_file(src_file), m_target_mailbox(dst_mbox) {}

        btpin_action::btpin_action(const std::string &client_mac, const std::string &target_pin)
                : m_client_mac(client_mac), m_target_pin(target_pin){        }


        std::vector<uint8_t> read_buffer(const std::string &path) {
            // open
            std::ifstream in;
            in.open(path, std::ios::binary | std::ios::in);

            // get size
            in.seekg(0, std::ios::end);
            auto end = in.tellg();
            in.seekg(0, std::ios::beg);
            auto beg = in.tellg();

            // read
            std::vector<uint8_t> result(static_cast<unsigned long>(end - beg));
            in.read(reinterpret_cast<char *>(result.data()), result.size());

            // validate
            if (in.bad()) {
                throw std::runtime_error(_("Cannot read file"));
            }
            return result;
        }

        void write_buffer(const std::string &path, std::vector<uint8_t> &&data) {
            // open
            std::ofstream out;
            out.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
            // write
            out.write(reinterpret_cast<const char *>(data.data()), data.size());
            // validate
            if (out.bad()) {
                throw std::runtime_error(_("Cannot write file"));
            }
        }

        void upload_action::perform(context &ctx) {
            uint8_t hnd = transfer(ctx);
            close(ctx, hnd);
            beep(ctx);

            std::cout << std::endl;
            std::cout << _("Done.") << std::endl;
        }

        uint8_t upload_action::transfer(context &ctx) {
            using namespace ev3loader::commands;
            request_download rq;
            rq.path = m_to;
            rq.contents = read_buffer(m_from);
            auto future = async_command<download_command>(*ctx.connection, rq, progresscallback{progress_report});
            future.wait();
        }

        void upload_action::close(context &ctx, uint8_t handle) {
            using namespace ev3loader::commands;
            request_handle_close rq;
            rq.fd = handle;
            auto close = async_command<closehandle_command>(*ctx.connection, rq);
            close.wait();
        }

        void upload_action::beep(context &ctx) {
            using namespace ev3loader::commands;
            std::vector<uint8_t> beep_code = {0x82, 0x0f, 0x0a};
            request_direct rq;
            rq.globals = 0;
            rq.locals = 0;
            rq.type = direct_command_type::WITHOUT_REPLY;
            rq.bytecode = buffer_view<>{beep_code};
            async_command<direct_command>(*ctx.connection, rq);
        }

        void download_action::perform(context &ctx) {
            using namespace ev3loader::commands;
            request_upload rq;
            rq.path = m_from;
            rq.op = upload_type::regular;
            auto future = async_command<upload_command>(*ctx.connection, rq, progresscallback{progress_report});
            auto result = future.get();
            write_buffer(m_to, std::move(result.file_payload));

            request_handle_close rq2;
            rq2.fd = result.file_id;
            auto close = async_command<closehandle_command>(*ctx.connection, rq2);
            close.wait();

            std::cout << std::endl;
            std::cout << _("Done.") << std::endl;
        }

        void ls_action::perform(context &ctx) {
            using namespace ev3loader::commands;

            std::cout << format(_("Asking for '%s' contents..."), m_dir.c_str()) << std::endl;
            request_ls in = {this->m_dir};
            auto future = async_command<ls_command>(*ctx.connection, in);
            auto out = future.get();

            std::cout << format(_("Directory listing for '%s':"), m_dir.c_str()) << std::endl;
            for (auto &&elem : out.listing) {
                std::cout << (elem.what == file_type::directory ? "dir " : "fil ");
                std::cout << std::setw(10) << std::setfill(' ') << std::dec << elem.size << std::setw(0);
                std::cout << " " << elem.name << (elem.what == file_type::directory ? "/" : "");
                if (m_md5 && elem.what == file_type::file) {
                    std::cout << " [";
                    std::cout << std::hex << std::setfill('0');
                    for (uint8_t byte : elem.md5) {
                        std::cout << std::setw(2) << (int) byte;
                    }
                    std::cout << std::setw(0) << "]";
                }
                std::cout << std::endl;
            }
            std::cout << _("Done.") << std::endl;
        }


        void rm_action::perform(context &ctx) {
            using namespace ev3loader::commands;
            request_fileop rq;
            rq.path = m_path;
            rq.what = fileop_type::unlink;
            auto future = async_command<fileop_command>(*ctx.connection, rq);
            auto result = future.get();
            std::cout << _("Done.") << std::endl;
        }


        void mkdir_action::perform(context &ctx) {
            using namespace ev3loader::commands;
            request_fileop rq;
            rq.path = m_path;
            rq.what = fileop_type::mkdir;
            auto future = async_command<fileop_command>(*ctx.connection, rq);
            auto result = future.get();
            std::cout << _("Done.") << std::endl;
        }


        void closehandle_action::perform(context &ctx) {
            using namespace ev3loader::commands;

            for (int i = 0; i < 256; i++) {
                closehandle_single_action{(file_handle) i}.perform(ctx);
                progress_report(i + 1, 256);
            }
            std::cout << std::endl << _("Done.") << std::endl;
        }


        void version_action::perform(context &ctx) {
            std::cout << format("ev3tool %s", TOOL_VERSION) << std::endl;
        }

        help_action::help_action(const arglist &flags, const arglist &commands)
                : m_flags(flags), m_commands(commands) {}

        void help_action::perform(context &ctx) {
            std::cout << format(_("EV3 CLI tool %s"), TOOL_VERSION) << std::endl;
            std::cout << _("Usage:") << std::endl;
            std::cout << _("ev3tool [flags] <command> [flags]") << std::endl;
            for (auto &&str : m_flags) {
                std::cout << str << std::endl;
            }
            std::cout << std::endl;
            std::cout << _("Commands:") << std::endl;
            for (auto &&str : m_commands) {
                std::cout << str << std::endl;
            }
            std::cout << std::endl;
            std::cout << _("Address syntax:") << std::endl;
            std::cout << _("HID:<VID>:<PID>[:<SN>]") << std::endl;
            std::cout << _("  VID = USB/BT hexadecimal vendor id") << std::endl;
            std::cout << _("  PID = USB/BT hexadecimal product id") << std::endl;
            std::cout << _("  SN  = USB/BT product serial number") << std::endl;
        }


        void hid_connect::perform(context &ctx) {
            auto addr = find_address();
            auto link = std::make_unique<ev3loader::hidapi_link>(addr);
            ctx.connection = std::make_unique<ev3loader::ev3_connection>(std::move(link));
            ctx.connection->default_callback(ev3loader::packet_callback{
                    [](const bytearray &pkt) {
                        std::cerr << _("- unexpected packet -") << std::endl;
                    },
                    [&ctx](std::exception_ptr ex) {
                        std::cerr << _("Error on the receive thread!") << std::endl;
                        try {
                            std::rethrow_exception(std::move(ex));
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
                        std::exit(1);
                    }
            });
        }


        void hid_list::perform(context &ctx) {
            std::cout << _("HID devices matching EV3:") << std::endl;
            hidapi_discovery search;

            hid_address normal(EV3_VID, EV3_PID);
            hid_address update(EV3_VID, EV3_PID_FWMODE);

            auto list = search.discover(normal);
            auto updList = search.discover(update);
            list.insert(list.end(), updList.begin(), updList.end());

            auto it = list.begin();
            auto end = list.end();
            int count = 0;

            for (; it != end; ++it, count++) {
                std::string fmt;
                if (it->product_id == EV3_PID) {
                    fmt = _("- '%s' -> EV3 lms2012");
                } else if (it->product_id == EV3_PID_FWMODE) {
                    fmt = _("- '%s' -> EV3 firmware update");
                } else {
                    fmt = _("- '%s' -> EV3 unknown");
                }
                std::string addr = (std::string) *it;
                std::cout << format(fmt.c_str(), addr.c_str()) << std::endl;
                std::cout << std::endl;
            }

            std::string fmt = _num("Found total %d brick.", "Found total %d bricks.", count);
            std::cout << format(fmt.c_str(), count) << std::endl;
        }

        ev3loader::hid_address hid_connect::find_address() {
            hidapi_discovery search;

            auto list = search.discover(m_address);
            auto it = list.begin();
            auto end = list.end();
            for (; it != end; ++it) {
                if (it->product_id == m_address.product_id &&
                    it->vendor_id == m_address.vendor_id &&
                    (m_address.serial_no.empty() || it->serial_no == m_address.serial_no)) {
                    return *it;
                }
            }
            throw ev3loader::hidapi_error(m_address, hid_errorcode::CONNECTION_FAILED, L"Cannot find device", HERE);
        }


        void fwenter_action::perform(context &ctx) {
            async_command<ev3loader::commands::fwupdate_command>(*ctx.connection);
        }

        void fwexit_action::perform(context &ctx) {
            using namespace ev3loader::commands;
            std::cout << _("Rebooting EV3...") << std::endl;

            auto future = async_command<fwstartapp_command>(*ctx.connection);
            future.get();

            std::cout << _("Reboot started.") << std::endl;
        }

        void fwdl_action::perform(context &ctx) {
            this->m_ctx = &ctx;

            handle_file();
            handle_hwversion();
            handle_erase();
            handle_flash();
            handle_checksum();
            handle_reboot();
        }

        void fwdl_action::handle_file() {
            std::cout << _("Loading firmware file...") << std::endl;
            m_fw = read_buffer(m_from);
            uLong crc = crc32(0L, Z_NULL, 0L);
            crc = crc32(crc, m_fw.data(), m_fw.size());
            m_fw_crc32 = crc;
            std::cout << format(_("Firmware loaded. Length %d, CRC32 0x%08x"), m_fw.size(), m_fw_crc32) << std::endl;
        }

        void fwdl_action::handle_hwversion() {
            fwversion_action{}.perform(*m_ctx);
        }

        void fwdl_action::handle_erase() {
            using namespace ev3loader::commands;

            std::cout << _("Erasing flash...") << std::endl;
            auto eraseFuture = async_command<fwerase_command>(*m_ctx->connection);
            eraseFuture.get();
            std::cout << _("Erasing finished.") << std::endl;
        }

        void fwdl_action::handle_flash() {
            using namespace ev3loader::commands;

            std::cout << _("Flashing new firmware...") << std::endl;
            request_fwdl req = {};
            req.address = 0x00000000;
            req.firmware = m_fw;
            auto uploadFuture = async_command<fw_download>(*m_ctx->connection, req, progresscallback{progress_report});
            uploadFuture.get();
            std::cout << std::endl << _("Flashing finished.") << ::std::endl;
        }

        void fwdl_action::handle_checksum() {
            using namespace ev3loader::commands;

            std::cout << _("Verifying firmware integrity...") << std::endl;
            request_checksum req = {};
            req.address = 0x00000000;
            req.length = m_fw.size();
            auto crcFuture = async_command<fwchecksum_command>(*m_ctx->connection, req);
            reply_checksum csum = crcFuture.get();

            if (csum.crc32 == m_fw_crc32) {
                std::cout << format(_("CRC32 matches: 0x%08x"), csum.crc32) << std::endl;
            } else {
                std::cout << format(
                        _("CRC32 does not match (0x%08x remote, 0x%08x local)"),
                        csum.crc32,
                        m_fw_crc32) << std::endl;
                throw communication_error(_("CRC32 mismatch"), HERE);
            }
        }

        void fwdl_action::handle_reboot() {
            fwexit_action{}.perform(*m_ctx);
        }

        void fwversion_action::perform(context &ctx) {
            using namespace ev3loader::commands;
            auto verFuture = async_command<fwversion_command>(*ctx.connection);
            fwversion ver = verFuture.get();

            char hwMajor = static_cast<char>('0' + (ver.hardwareVersion / 10) % 10);
            char hwMinor = static_cast<char>('0' + (ver.hardwareVersion / 1) % 10);
            char fwMajor = static_cast<char>('0' + (ver.firmwareVersion / 10) % 10);
            char fwMinor = static_cast<char>('0' + (ver.firmwareVersion / 1) % 10);

            std::cout << format(_("Hardware version: V%c.%c0"), hwMajor, hwMinor) << std::endl;
            std::cout << format(_("EEPROM   version: V%c.%c0"), fwMajor, fwMinor) << std::endl;
        }

        void closehandle_single_action::perform(context &ctx) {
            using namespace ev3loader::commands;

            request_handle_close rq = {m_handle};

            auto future = async_command<closehandle_command>(*ctx.connection, rq);
            future.wait();
        }

        void listhandle_action::perform(context &ctx) {
            throw communication_error(_("File handle listing is not yet implemented."), HERE);
        }

        void mailbox_action::perform(context &ctx) {
            using namespace ev3loader::commands;

            std::cout << _("Reading message...") << std::endl;
            bytearray msg = read_buffer(m_source_file);

            std::cout << _("Writing mailbox...") << std::endl;
            request_mailbox rq = {};
            rq.name = m_source_file;
            rq.payload = std::move(msg);

            auto future = async_command<write_mailbox_command>(*ctx.connection, rq);
            future.wait();

            std::cout << _("Done.") << std::endl;
        }

        void btpin_action::perform(context &ctx) {
            using namespace ev3loader::commands;

            request_setpin rq = {};
            rq.brickPin = (uint32_t)std::stoul(m_target_pin, nullptr, 10);
            int n = std::sscanf(m_client_mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                    rq.clientMac+0, rq.clientMac+1, rq.clientMac+2,
                    rq.clientMac+3, rq.clientMac+4, rq.clientMac+5);
            if (n != 6) {
                throw communication_error(_("Invalid format of the provided MAC address."), HERE);
            }

            std::cout << _("Sending pin change request...") << std::endl;
            auto future = async_command<bluetooth_pin_command>(*ctx.connection, rq);
            auto result = future.get();

            std::cout << _("Success! Brick blutooth info:") << std::endl;
            std::cout << format(_("- brick PIN: %u"), result.brickPin) << std::endl;
            std::cout << format(_("- brick MAC: %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX"),
                    result.brickMac[0], result.brickMac[1], result.brickMac[2],
                    result.brickMac[3], result.brickMac[4], result.brickMac[5]) << std::endl;
        }
    }
}
