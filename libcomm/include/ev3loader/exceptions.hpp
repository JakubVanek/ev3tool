//
// Created by kuba on 29.1.18.
//

#ifndef LIBEV3COMM_EXCEPTIONS_HPP
#define LIBEV3COMM_EXCEPTIONS_HPP

#include <sstream>
#include <utility>
#include <ev3loader/common.hpp>
#include <ev3loader/utils/utils.hpp>
#include <ev3loader/ev3proto.hpp>

namespace ev3loader {
    enum class ExtraPurpose {
        None,
        PacketId,
    };

    class communication_error : public std::exception {
    public:
        communication_error()
                : location(), description(), extra(-1), extrawut(ExtraPurpose::None) {}

        explicit
        communication_error(i18n::locstring desc)
                : location(), description(std::move(desc)), extra(-1), extrawut(ExtraPurpose::None) {}

        communication_error(i18n::locstring desc, std::string loc)
                : location(std::move(loc)), description(std::move(desc)), extra(-1), extrawut(ExtraPurpose::None) {}

        communication_error(i18n::locstring desc, std::string loc, intptr_t extra, ExtraPurpose wut)
                : location(std::move(loc)), description(std::move(desc)), extra(extra), extrawut(wut) {}

        communication_error &setLocation(std::string const &loc) {
            location = loc;
            return *this;
        }

        communication_error &setDescription(i18n::locstring const &desc) {
            description = desc;
            return *this;
        }

        communication_error &setExtra(intptr_t e, ExtraPurpose wut) {
            extra = e;
            extrawut = wut;
            return *this;
        }

        const std::string &getLocation() const {
            return location;
        }

        const i18n::locstring &getDescription() const {
            return description;
        }

        intptr_t getExtra() const {
            return extra;
        }

        ExtraPurpose getExtraPurpose() const {
            return extrawut;
        }

        const char *what() const noexcept override {
            return getDescription().c_str();
        }

        virtual operator std::exception_ptr() const {
            return std::make_exception_ptr(*this);
        }

    private:
        std::string location;
        i18n::locstring description;
        intptr_t extra;
        ExtraPurpose extrawut;
    };

    class transmit_failed : public communication_error {
    public:
        transmit_failed()
                : communication_error() {}

        transmit_failed(pcnt_t packet_id, i18n::locstring desc, std::string &&where)
                : communication_error(std::move(desc), std::move(where), packet_id, ExtraPurpose::PacketId) {}

        operator std::exception_ptr() const override {
            return std::make_exception_ptr(*this);
        }
    };

    class abort_error : public communication_error {
    public:
        abort_error() : communication_error() {}

        explicit
        abort_error(i18n::locstring desc)
                : communication_error(std::move(desc)) {}

        abort_error(i18n::locstring desc, std::string where)
                : communication_error(std::move(desc), std::move(where)) {}

        abort_error(i18n::locstring desc, std::string where, intptr_t extra, ExtraPurpose wut)
                : communication_error(std::move(desc), std::move(where), extra, wut) {}

        operator std::exception_ptr() const override {
            return std::make_exception_ptr(*this);
        }
    };

    class timeout_error : public communication_error {
    public:
        timeout_error() : communication_error() {}

        explicit
        timeout_error(i18n::locstring desc)
                : communication_error(std::move(desc)) {}

        timeout_error(i18n::locstring desc, std::string where)
                : communication_error(std::move(desc), std::move(where)) {}

        timeout_error(i18n::locstring desc, std::string where, intptr_t extra, ExtraPurpose wut)
                : communication_error(std::move(desc), std::move(where), extra, wut) {}

        operator std::exception_ptr() const override {
            return std::make_exception_ptr(*this);
        }
    };

    class parse_failed : public communication_error {
    public:
        parse_failed()
                : code(parse_error::UNKNOWN_PARSE_ERROR), data(0) {}

        explicit
        parse_failed(parse_error err)
                : code(err), data(0) {}

        parse_failed(parse_error err, intptr_t dat)
                : code(err), data(dat) {}

        const char *what() const noexcept override {
            try {
                std::string str = parse_error_desc(code);
                const char *data = strdup(str.c_str());
                return data;
            } catch (...) {
                return "FATAL EXCEPTION ERROR";
            }
        }

        parse_error get_code() const {
            return code;
        }

        intptr_t get_data() const {
            return data;
        }

        operator std::exception_ptr() const override {
            return std::make_exception_ptr(*this);
        }

    private:
        parse_error code;
        intptr_t data;
    };

    class sys_remote_error : public communication_error {
    public:
        sys_remote_error()
                : cmd(system_commands::COMMAND_ERROR), err(system_status::UNKNOWN_ERROR), data(0) {}

        sys_remote_error(system_commands cmd, system_status err)
                : cmd(cmd), err(err), data(0) {}

        sys_remote_error(system_commands cmd, system_status err, intptr_t data)
                : cmd(cmd), err(err), data(data) {}

        const char *what() const noexcept override {
            try {
                return strdup(system_status_desc(err).c_str());
            } catch (...) {
                return "FATAL EXCEPTION ERROR";
            }
        }

        system_commands get_command() const {
            return cmd;
        }

        system_status get_status() const {
            return err;
        }

        intptr_t get_data() const {
            return data;
        }

        operator std::exception_ptr() const override {
            return std::make_exception_ptr(*this);
        }

    private:
        system_commands cmd;
        system_status err;
        intptr_t data;
    };
}

#endif //LIBEV3COMM_EXCEPTIONS_HPP
