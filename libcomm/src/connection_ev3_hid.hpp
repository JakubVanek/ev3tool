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

#include "connection.hpp"
#include "config.hpp"
#include "connection_ev3_hid_types.hpp"
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <hidapi/hidapi.h>

namespace ev3loader {

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


        /**
         * Copy link struct.
         * @param other Struct to copy.
         */
        hidapi_link(const hidapi_link &other) = delete;

        /**
         * Move link struct.
         * @param other Struct to move.
         */
        hidapi_link(hidapi_link &&other) = default;

        /**
         * Copy link struct.
         * @param other Struct to copy.
         * @return This struct.
         */
        hidapi_link &operator=(const hidapi_link &other) = delete;

        /**
         * Move link struct.
         * @param other Struct to move.
         * @return This struct.
         */
        hidapi_link &operator=(hidapi_link &&other) = default;

        void write(nativewindow &in) override;

        void read(nativewindow &out, std::chrono::milliseconds timeout) override;

        bool nonblock_check() const override;

        void nonblock_set(bool enabled) override;

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
        /**
         * Nonblock status.
         */
        bool m_nonblock = false;
    };

    /**
     * EV3 HIDAPI searcher.
     */
    class hidapi_discovery final : public device_discovery<hid_address> {
    public:
        /**
         * Create new searcher.
         */
        hidapi_discovery();

        /**
         * Does nothing.
         */
        ~hidapi_discovery() override;

        /**
         * Search for EV3 connected through USB or Bluetooth.
         * @return List of available addresses.
         */
        device_list discover() override;

        /**
         * Search for EV3 connected through USB or Bluetooth.
         * @param addr Partial HID address (only VID & PID) or nullptr.
         * @return List of available addresses.
         */
        device_list discover(const hid_address &addr) override;
    };
}

#endif//EV3_CONNECTION_HID_HPP
