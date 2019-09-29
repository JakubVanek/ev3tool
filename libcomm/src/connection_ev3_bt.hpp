/**
 * EV3loader - the EV3 uploader
 * 
 * Copyright (C) 2017  Faculty of Electrical Engineering, CTU in Prague
 * Author: Jakub Vanek <nxc4ev3@vankovi.net>
 * 
 * 
 * EV3 HID communcation interface.
 */

//#ifndef EV3_CONNECTION_BT_HPP
#if 0
#define EV3_CONNECTION_BT_HPP

#include "connection.hpp"
#include "config.hpp"
#include "connection_ev3_bt_types.hpp"
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace ev3loader {
    /**
     * HIDAPI channel.
     */
    class rfcomm_link : public channel {
    public:
        /**
         * Create new empty link.
         */
        rfcomm_link() = delete;

        /**
         * Create new link.
         * @param addr USB address.
         */
        explicit rfcomm_link(rfcomm_address addr);

        ~rfcomm_link();


        /**
         * Copy link struct.
         * @param other Struct to copy.
         */
        rfcomm_link(const rfcomm_link &other) = delete;

        /**
         * Move link struct.
         * @param other Struct to move.
         */
        rfcomm_link(rfcomm_link &&other) = default;

        /**
         * Copy link struct.
         * @param other Struct to copy.
         * @return This struct.
         */
        rfcomm_link &operator=(const rfcomm_link &other) = delete;

        /**
         * Move link struct.
         * @param other Struct to move.
         * @return This struct.
         */
        rfcomm_link &operator=(rfcomm_link &&other) = default;

        void write(nativewindow &in) override;

        void read(nativewindow &out, std::chrono::milliseconds timeout) override;

        bool nonblock_check() const override;

        void nonblock_set(bool enabled) override;

    private:
        /**
         * Local socket.
         */
        int m_sock = -1;
        /**
         * Device socket.
         */
        int m_dev = -1;
        /**
         * Device address.
         */
        rfcomm_address m_addr;
        /**
         * Nonblock status.
         */
        bool m_nonblock = false;
    };

    /**
     * EV3 BT searcher.
     */
    class rfcomm_discovery final : public device_discovery<rfcomm_address> {
    public:
        /**
         * noop
         */
        device_list discover() override {
            return device_list{};
        }

        /**
         * noop
         */
        device_list discover(const rfcomm_address &addr) override {
            return device_list{};
        }
    };
}

#endif//EV3_CONNECTION_BT_HPP
