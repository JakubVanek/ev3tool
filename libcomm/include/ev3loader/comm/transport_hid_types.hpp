//
// Created by kuba on 22.7.18.
//

#ifndef LIBEV3COMM_CONNECTION_EV3_USB_TYPES_HPP
#define LIBEV3COMM_CONNECTION_EV3_USB_TYPES_HPP

#include <cstdint>
#include <string>
#include <algorithm>
#include <ev3loader/config.hpp>

namespace ev3loader {
    namespace comm {
/**
 * USB address.
 */
        struct hid_address {
            /**
             * Create new empty USB address.
             */
            hid_address()
                    : vendor_id(0), product_id(0) {}

            /**
             * Create new partial USB address.
             * @param vid Device Vendor ID.
             * @param pid Device Product ID.
             */
            explicit hid_address(uint16_t vid, uint16_t pid)
                    : vendor_id(vid), product_id(pid) {}

            /**
             * Create new USB address.
             * @param str HID address string.
             */
            explicit hid_address(std::string str);

            /**
             * Create new full USB address.
             * @param vid Device Vendor ID.
             * @param pid Device Product ID.
             * @param sn  Device Serial Number.
             */
            explicit hid_address(uint16_t vid, uint16_t pid, std::wstring sn)
                    : vendor_id(vid), product_id(pid), serial_no(std::move(sn)) {}

            /**
             * Copy an USB address.
             * @param other Original address.
             */
            hid_address(const hid_address &other) = default;

            /**
             * Move an USB address.
             * @param other Original address.
             */
            hid_address(hid_address &&other) = default;

            /**
             * Copy an USB address.
             * @param other Original address.
             * @return
             */
            hid_address &operator=(const hid_address &other) = default;

            /**
             * Move an USB address.
             * @param other Original address.
             * @return
             */
            hid_address &operator=(hid_address &&other) = default;

            /**
             * Convert USB address to a string.
             * @return String description of the address.
             */
            explicit operator std::string() const;

            bool is_ev3_normal() const {
                return vendor_id == EV3_VID && product_id == EV3_PID;
            }

            bool is_ev3_fwmode() const {
                return vendor_id == EV3_VID && product_id == EV3_PID_FWMODE;
            }

            /**
             * USB Vendor ID.
             */
            uint16_t vendor_id;
            /**
             * USB Product ID.
             */
            uint16_t product_id;
            /**
             * USB Serial Number descriptor.
             */
            std::wstring serial_no;

            // informational only
            std::wstring product_name;
            std::wstring vendor_name;
        };
    }
}
#endif //LIBEV3COMM_CONNECTION_EV3_USB_TYPES_HPP
