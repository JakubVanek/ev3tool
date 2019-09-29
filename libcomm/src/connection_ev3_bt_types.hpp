//
// Created by kuba on 22.7.18.
//

//#ifndef LIBEV3COMM_CONNECTION_EV3_BT_TYPES_HPP
#if 0
#define LIBEV3COMM_CONNECTION_EV3_BT_TYPES_HPP

#include <cstdint>
#include <string>
#include <algorithm>
#include <utility>

namespace ev3loader {
/**
 * USB address.
 */
struct rfcomm_address {
    /**
     * Create new empty BT address.
     */
    rfcomm_address()
            : mac_address{0x00, 0x00, 0x00, 0x00, 0x00, 0x00} {}

    /**
     * Create new BT address.
     * @param vid Device Vendor ID.
     * @param pid Device Product ID.
     */
    explicit rfcomm_address(const std::string& address);

    /**
     * Copy an USB address.
     * @param other Original address.
     */
    rfcomm_address(const rfcomm_address &other) = default;

    /**
     * Move an USB address.
     * @param other Original address.
     */
    rfcomm_address(rfcomm_address &&other) = default;

    /**
     * Copy an USB address.
     * @param other Original address.
     * @return
     */
    rfcomm_address &operator=(const rfcomm_address &other) = default;

    /**
     * Move an USB address.
     * @param other Original address.
     * @return
     */
    rfcomm_address &operator=(rfcomm_address &&other)      = default;

    /**
     * Convert USB address to a string.
     * @return String description of the address.
     */
    explicit operator std::string() const;

    /**
     * USB Vendor ID.
     */
    uint8_t mac_address[6];
};
}
#endif //LIBEV3COMM_CONNECTION_EV3_BT_TYPES_HPP
