/**
 * EV3loader - the EV3 uploader
 * 
 * Copyright (C) 2017  Faculty of Electrical Engineering, CTU in Prague
 * Author: Jakub Vanek <nxc4ev3@vankovi.net>
 * 
 * 
 * EV3 HID communcation interface.
 */

#ifndef EV3_CONNECTION_HID_HPP
#define EV3_CONNECTION_HID_HPP

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <hidapi/hidapi.h>

#include <ev3loader/comm/base.hpp>
#include <ev3loader/comm/transport_hid_types.hpp>
#include <ev3loader/config.hpp>

namespace ev3loader {
    namespace comm {
        typedef std::vector<hid_address> hid_addresses;

        struct hid_deleter {
            void operator()(hid_device *ptr) const noexcept {
                if (ptr) {
                    hid_close(ptr);
                }
            }
        };

        /**
         * HIDAPI channel.
         */
        class hidapi_link : public channel {
        public:
            /**
             * Create new empty link.
             */
            hidapi_link() = delete;

            /**
             * Create new link.
             * @param addr USB address.
             */
            explicit hidapi_link(hid_address addr);

            // no copy constructors
            hidapi_link(const hidapi_link &other) = delete;
            hidapi_link &operator=(const hidapi_link &other) = delete;

            // allow move constructors
            hidapi_link(hidapi_link &&other) = default;
            hidapi_link &operator=(hidapi_link &&other) = default;

            void write(nativewindow &in,
                       milliseconds timeout,
                       bool blocking) override;

            void read(nativewindow &out,
                      milliseconds timeout,
                      bool blocking) override;

            /**
             * Search for EV3 connected through USB or Bluetooth.
             * @return List of available addresses.
             */
            static hid_addresses discover();

        private:
            using devptr_t = std::unique_ptr<hid_device, hid_deleter>;

            /**
             * Device handle.
             */
            devptr_t m_dev;
            /**
             * Device address.
             */
            hid_address m_addr;
        };
    }
}

#endif//EV3_CONNECTION_HID_HPP
