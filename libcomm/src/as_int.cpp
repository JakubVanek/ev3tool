//
// Created by kuba on 28.7.18.
//

#include "as_int.hpp"
#include "bytewriter.hpp"

namespace ev3loader {
    as_system_header::as_system_header(message_type type, system_commands cmd)
            : m_type(type), m_command(cmd) {}

    void as_system_header::doIt(bytewriter &out) const {
        out << as_size(0);
        out << as_id(0);
        out << as_type(m_type);
        out << as_command(m_command);
    }

    as_system_header::~as_system_header() = default;
}