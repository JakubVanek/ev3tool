//
// Created by kuba on 30.7.18.
//

#include "cmdline_parser.hpp"
#include "localization_tool.hpp"
#include "actions.hpp"

#include <connection_ev3_hid.hpp>
#include <connection_ev3.hpp>
#include <commands.hpp>

#include <fstream>
#include <iomanip>

using namespace ev3loader;
using namespace ev3tool::i18n;

namespace ev3tool {
    namespace program {

        action::~action() = default;

        bootstrap::~bootstrap() = default;

        composite_action::composite_action() = default;

        composite_action::~composite_action() = default;


        void composite_action::append(std::unique_ptr<action> action) {
            m_list.push_back(std::move(action));
        }

        void composite_action::clear() {
            m_list.clear();
        }

        void composite_action::perform(context &ctx) {
            auto it = m_list.begin();
            auto end = m_list.end();

            for (; it != end; ++it) {
                (*it)->perform(ctx);
            }
        }

        ev3tool_bootstrap::ev3tool_bootstrap() : m_lvl(OUTPUT_NORMAL) {
            add_counter_arg("help", "-h", "--help", +1, _("Show this help message."));
            add_counter_arg("version", "-V", "--version", +1, _("Show program version string."));
            add_counter_arg("verbose", "-v", "--verbose", +1, _("Increase output verbosity."));
            add_counter_arg("verbose", "-q", "--quiet", -1, _("Decrease output verbosity."));
            add_counter_arg("md5", "", "--md5", +1, _("Show MD5 hashes of files in directory listings."));
            add_string_arg("connect", "-c", "--connect", _("Connect to EV3 at a specific address."));
            add_string_arg("from", "-f", "--from", _("Upload/download file from a location."));
            add_string_arg("to", "-t", "--to", _("Upload/download file to a location."));
            add_cmd_arg("list", "list", _("Prints all connected bricks."));
            add_cmd_arg("upload", "upload", _("Upload a file to the brick. Needs --from and --to."));
            add_cmd_arg("download", "download", _("Download a file from the brick. Needs --from and --to."));
            add_cmd_arg("mkdir", "mkdir", _("Creates a directory on the brick. Needs --to."));
            add_cmd_arg("ls", "ls", _("Print directory listing from the brick. Needs --from."));
            add_cmd_arg("rm", "rm", _("Remove a file from the brick. Needs --from."));
            add_cmd_arg("mailbox", "mailbox", _("Write into EV3 mailbox. Needs --from (file) and --to (mailbox name)."));
            add_cmd_arg("btpin", "btpin", _("Set BT pin. Needs --from (client MAC) and --to (new PIN)."));
            add_cmd_arg("listhnds", "listhnds", _("List all open handles."));
            add_cmd_arg("closehnds", "closehnds", _("Try to close all handles."));
            add_cmd_arg("closehnd", "closehnd", _("Close only one handle. Needs --from."));
            add_cmd_arg("fwenter", "fwenter", _("Reboot into bootloader/firmware update mode."));
            add_cmd_arg("fwexit", "fwexit", _("Reboot out of bootloader/firmware update mode."));
            add_cmd_arg("fwver", "fwver", _("Get HW/FW version from bootloader mode."));
            add_cmd_arg("fwflash", "fwflash", _("Flash a firmware file to the brick. Needs --from."));

            add_syntesizer("verbose", [this](argument_storage &args) {
                int level = std::stoi(args.at("verbose"));
                m_lvl = level < 0 ? OUTPUT_QUIET :
                        level > 0 ? OUTPUT_VERBOSE :
                        OUTPUT_NORMAL;
                return loop_continue;
            });

            add_syntesizer("help", [this](argument_storage &) {
                m_root = nullptr;
                return loop_break;
            });

            add_syntesizer("version", [this](argument_storage &) {
                m_root = std::make_unique<version_action>();
                return loop_break;
            });

            add_syntesizer("list", [this](argument_storage &) {
                m_root = std::make_unique<hid_list>();
                return loop_break;
            });

            add_syntesizer("mkdir", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<mkdir_action>(args.at("to")));
                return loop_break;
            });

            add_syntesizer("upload", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<upload_action>(args.at("from"), args.at("to")));
                return loop_break;
            });

            add_syntesizer("download", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<download_action>(args.at("from"), args.at("to")));
                return loop_break;
            });

            add_syntesizer("ls", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<ls_action>(args.at("from"), args.find("md5") != args.end()));
                return loop_break;
            });

            add_syntesizer("rm", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<rm_action>(args.at("from")));
                return loop_break;
            });

            add_syntesizer("listhnds", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<listhandle_action>());
                return loop_break;
            });

            add_syntesizer("closehnds", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<closehandle_action>());
                return loop_break;
            });

            add_syntesizer("closehnd", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<closehandle_single_action>(std::stoi(args.at("from"))));
                return loop_break;
            });

            add_syntesizer("mailbox", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<mailbox_action>(args.at("from"), args.at("to")));
                return loop_break;
            });

            add_syntesizer("btpin", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<btpin_action>(args.at("from"), args.at("to")));
                return loop_break;
            });

            add_syntesizer("fwenter", [this](argument_storage &args) {
                setup_connect(args);
                append_composite(std::make_unique<fwenter_action>());
                return loop_break;
            });

            add_syntesizer("fwexit", [this](argument_storage &args) {
                setup_connect(args, EV3_VID, EV3_PID_FWMODE);
                append_composite(std::make_unique<fwexit_action>());
                return loop_break;
            });

            add_syntesizer("fwver", [this](argument_storage &args) {
                setup_connect(args, EV3_VID, EV3_PID_FWMODE);
                append_composite(std::make_unique<fwversion_action>());
                return loop_break;
            });

            add_syntesizer("fwflash", [this](argument_storage &args) {
                setup_connect(args, EV3_VID, EV3_PID_FWMODE);
                append_composite(std::make_unique<fwdl_action>(args.at("from")));
                return loop_break;
            });
        }

        std::unique_ptr<action> ev3tool_bootstrap::take_action() {
            return std::move(this->m_root);
        }

        output_level ev3tool_bootstrap::get_verbosity() {
            return m_lvl;
        }


        void ev3tool_bootstrap::process(int argc, char **argv) {
            auto &&vec = stringize(argc, argv);
            auto &&stor = analyze(vec);
            synthesize(stor);
        }

        argument_list ev3tool_bootstrap::stringize(int argc, char **argv) {
            argument_list vector(static_cast<unsigned long>(argc));
            std::copy(argv, argv + argc, vector.begin());
            return vector;
        }

        argument_storage ev3tool_bootstrap::analyze(argument_list &args) {
            argument_storage stor;

            for (int i = 0; i < args.size(); i++) {
                for (auto &&handler : m_flags) {
                    if (handler->process(args, i, stor) == loop_break) {
                        break;
                    }
                }
            }
            return stor;
        }

        void ev3tool_bootstrap::synthesize(argument_storage &args) {
            for (auto &&pair : args) {
                auto it = m_synths.find(pair.first);
                if (it != m_synths.end()) {
                    try {
                        if (it->second(args) == loop_break) {
                            break;
                        }
                    } catch (std::out_of_range &ex) {
                        std::cerr << _("Error: missing required arguments.") << std::endl;
                        break;
                    }
                }
            }
            if (m_root == nullptr) {
                m_root = std::make_unique<help_action>(m_flag_help, m_cmd_help);
            }
        }

        void ev3tool_bootstrap::setup_connect(argument_storage &args, int vid, int pid) {
            if (vid < 0)
                vid = EV3_VID;
            if (pid < 0)
                pid = EV3_PID;

            hid_address addr;
            auto it = args.find("connect");
            if (it == args.end()) {
                addr = hid_address{(uint16_t)vid, (uint16_t)pid};
            } else {
                addr = hid_address{it->second};
            }
            auto seq = std::make_unique<composite_action>();
            auto connect = std::make_unique<hid_connect>(addr);
            seq->append(std::move(connect));
            m_root = std::move(seq);
        }

        void ev3tool_bootstrap::append_composite(std::unique_ptr<action> act) {
            auto ptr = dynamic_cast<composite_action *>(m_root.get());
            if (ptr == nullptr) {
                throw std::logic_error("Cannot append to non-composite action");
            }
            ptr->append(std::move(act));
        }

        void ev3tool_bootstrap::add_counter_arg(const std::string &key, const std::string &shrt, const std::string &lng,
                                                int change, const std::string &desc) {
            m_flags.push_back(std::make_unique<counter_flag>(key, shrt, lng, change));
            push_normal_help(shrt, lng, desc, false);
        }

        void ev3tool_bootstrap::add_string_arg(const std::string &key, const std::string &shrt, const std::string &lng,
                                               const std::string &desc) {
            m_flags.push_back(std::make_unique<string_flag>(key, shrt, lng));
            push_normal_help(shrt, lng, desc, true);
        }

        void ev3tool_bootstrap::add_cmd_arg(const std::string &key, const std::string &name, const std::string &desc) {
            m_flags.push_back(std::make_unique<command_flag>(key, name));
            push_cmd_help(name, desc);
        }

        void ev3tool_bootstrap::push_normal_help(std::string shrt,
                                                 std::string lng,
                                                 const std::string &desc,
                                                 bool value) {
            if (value) {
                shrt += "=<>";
                lng += "=<>";
            }
            std::ostringstream line;

            const int ARG_WIDTH = 24;
            int offset;
            if (!shrt.empty() && !lng.empty()) {
                line << "  " << shrt << ", " << lng;
                offset = ARG_WIDTH - line.tellp();
                if (offset <= 0) {
                    line.str("");
                    line.clear();

                    line << "  " << shrt << ",\n " << lng;
                    offset = static_cast<int>(ARG_WIDTH - (lng.size() + 1));
                }
            } else {
                if (shrt.empty()) {
                    line << "      " << lng;
                } else {
                    line << "  " << shrt;
                }
                offset = ARG_WIDTH - line.tellp();
            }

            std::string spacer(static_cast<unsigned long>(offset), ' ');
            line << spacer << desc;

            m_flag_help.push_back(line.str());
        }

        void ev3tool_bootstrap::push_cmd_help(const std::string &name,
                                              const std::string &desc) {
            std::ostringstream line;
            line << std::left << std::setfill(' ');
            line << " " << std::setw(14) << name << desc;
            m_cmd_help.push_back(line.str());
        }

        void ev3tool_bootstrap::add_syntesizer(const std::string &key,
                                               std::function<loop_state(argument_storage &)> &&fn) {
            m_synths[key] = std::move(fn);
        }

        invalid_argument_exception::invalid_argument_exception()
                : m_reason(_("Invalid argument")) {}

        invalid_argument_exception::invalid_argument_exception(std::string &&str)
                : m_reason(std::move(str)) {}

        const char *invalid_argument_exception::what() const noexcept {
            return m_reason.c_str();
        }


        flag::flag(const std::string &key) : m_key(key) {}

        flag::~flag() = default;

        counter_flag::counter_flag(const std::string &key, const std::string &shrt, const std::string &lng, int change)
                : flag(key), m_short(shrt), m_long(lng), m_change(change) {}

        bool counter_flag::process(const argument_list &args, int &idx, argument_storage &stor) {
            auto &&arg = args[idx];

            bool ok_short = !m_short.empty() && arg == m_short;
            bool ok_long = !m_long.empty() && arg == m_long;

            if (ok_short || ok_long) {
                int current;
                auto it = stor.find(m_key);
                if (it == stor.end()) {
                    current = 0;
                } else {
                    current = std::stoi(stor[m_key], nullptr, 10);
                }
                stor[m_key] = std::to_string(current + m_change);
                return true;
            }
            return false;
        }


        string_flag::string_flag(const std::string &key, const std::string &shrt, const std::string &lng)
                : flag(key), m_short(shrt), m_long(lng) {}

        bool string_flag::process(const argument_list &args, int &idx, argument_storage &stor) {
            auto &&arg = args[idx];

            bool inline_ = false, twoarg = false;

            if (!m_short.empty()) {
                inline_ |= starts_with(arg, 0, m_short + "=");
                twoarg |= arg == m_short;
            }
            if (!m_long.empty()) {
                inline_ |= starts_with(arg, 0, m_long + "=");
                twoarg |= arg == m_long;
            }

            if (inline_) {
                size_t subidx = arg.find('=');
                stor[m_key] = std::string{arg.begin() + subidx + 1, arg.end()};
                return true;
            } else if (twoarg) {
                idx++;
                if (idx >= args.size()) {
                    throw invalid_argument_exception(
                            format(_("Missing argument for %s"), arg.c_str())
                    );
                }
                stor[m_key] = args[idx];
                return true;
            }
            return false;
        }

        command_flag::command_flag(const std::string &key, const std::string &name)
                : flag(key), m_name(name) {}

        bool command_flag::process(const argument_list &args, int &idx, argument_storage &stor) {
            auto &&arg = args[idx];

            if (arg == m_name) {
                auto it = stor.find("command_count");
                if (it == stor.end()) {
                    stor["command_count"] = "1";
                } else {
                    if (stoi(it->second, nullptr, 10) > 0) {
                        throw invalid_argument_exception(
                                format(_("Command already specified: %s"), arg.c_str()));
                    }
                }

                stor[m_key] = "1";
                return true;
            }
            return false;
        }

    }
}

