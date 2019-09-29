#if 0
//
// Created by kuba on 22.7.18.
//

#include <sstream>
#include <iomanip>
#include "utils.hpp"
#include "localization.hpp"
#include "connection_ev3_bt_types.hpp"


/////////////////
// HID ADDRESS //
/////////////////

using namespace ev3loader::i18n::priv;

namespace ev3loader {

    static void take_prefix(const std::string &str, size_t &idx) {
        if (!starts_with(str, idx, "bt:")) {
            throw std::invalid_argument(_("BT address does not contain valid prefix"));
        }
        idx += 3;
    }

    ev3loader::rfcomm_address::rfcomm_address(const std::string& addr)
            : mac_address{0x00, 0x00, 0x00, 0x00, 0x00, 0x00} {
        size_t idx = 0;
        take_prefix(addr, idx);
        int n = sscanf(addr.c_str() + idx,
                       "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                       mac_address + 0, mac_address + 1, mac_address + 2,
                       mac_address + 3, mac_address + 4, mac_address + 5);
        if (n != 6) {
            throw std::invalid_argument(_("BT address is invalid"));
        }
    }

    rfcomm_address::operator std::string() const {
        std::ostringstream str;

        str << "BT:" << format("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX",
                               mac_address[0], mac_address[1], mac_address[2],
                               mac_address[3], mac_address[4], mac_address[5]);

        return str.str();
    }
}
#endif
