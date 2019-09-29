/**
 * EV3loader - the EV3 uploader
 *
 * Copyright (C) 2017  Faculty of Electrical Engineering, CTU in Prague
 * Author: Jakub Vanek <nxc4ev3@vankovi.net>
 *
 *
 * Generic interfaces.
 */

#ifndef GENERIC_CONNECTION_HPP
#define GENERIC_CONNECTION_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <utility>
#include <vector>
#include "exceptions.hpp"
#include "common.hpp"
#include "callback.hpp"
#include "buffer.hpp"

namespace ev3loader {
    /**
     * Bidirectional channel.
     */
    class channel {
    public:
        /**
         * Virtual destructor.
         */
        virtual ~channel() = default;

        /**
         * Raw synchronous write.
         *
         * @param buffer Data to send.
         * @param length Data length.
         * @return Whether the transmission succeeded.
         */
        virtual void write(nativewindow &in) = 0;

        /**
         * Raw synchronous read.
         *
         * @param buffer Receive buffer.
         * @param length Maximum byte number.
         * @return Actual number of bytes that were read.
         */
        virtual void read(nativewindow &out, std::chrono::milliseconds timeout) = 0;

        /**
         * @copydoc ev3loader::packet_layer#nonblock_check() const
         */
        virtual bool nonblock_check() const = 0;

        /**
         * @copydoc ev3loader::packet_layer#nonblock_set(bool)
         */
        virtual void nonblock_set(bool enabled) = 0;
    };

    /**
     * <p>Low-level packet handler.</p>
     *
     * <p>This class is expected to operate on a bidirectional channel to desired device. It then
     * handles concurrent accesses to the channel.</p>
     *
     * <p>It is built upon an event loop model. The user has to periodically call the update() method to
     * check for received packets. Whether this call blocks is determined by @see {packet_layer#nonblock_set(bool)}
     * function. Blocking variant (the default) is suitable for models with separate event thread, whether
     * non-blocking variant is better suitable for single-threaded event loops.</p>
     *
     * <p>The interface provides these modes of operation:</p>
     * <ul>
     *   <li>Managed mode: synchronous write, reassembly & routing of received packets and asynchronous call back.
     *       This is the default mode.</li>
     *   <li>Exclusive mode: synchronous write, direct asynchronous call back.
     *       This mode is intended for sending and receiving of non-packet data.</li>
     * </ul>
     *
     * <p>Device address isn't defined by this interface, it's up to each implementation to decide
     * what device to connect to.</p>
     */
    class packet_layer {
    public:
        /**
         * <p>Virtual destructor.</p>
         */
        virtual ~packet_layer() = default;

        /**
         * <p>Send a packet to the device.</p>
         *
         * <p>This function writes the necessary accounting details and sends the packet through the channel.</p>
         *
         * @param pkt Transaction details. If the packet is supposed to generate a reply, @see {transaction#on_receive}
         *            should be set.
         * @return Status: \c true when operation succeeded, \c false otherwise.
         */
        virtual pcnt_t push_packet(transaction &pkt) = 0;

        /**
         * <p>Specify default callback for managed mode.</p>
         *
         * @param on_receive Function called on receive of any data.
         * @return Status: \c true when operation succeeded, \c false otherwise.
         */
        virtual void default_callback(packet_callback on_receive) = 0;
    };

    /**
     * <p>Device searcher.</p>
     *
     * <p>This interface supports both blocking and nonblocking modes.</p>
     *
     * @tparam Address     Device address type.
     */
    template<typename Address, typename ListType = std::vector<Address> >
    class device_discovery {
    public:
        /**
         * <p>List of discovered device addresses.</p>
         */
        typedef ListType device_list;

        /**
         * <p>Virtual destructor.</p>
         */
        virtual ~device_discovery() = default;

        /**
         * <p>Search for connected compatible devices.</p>
         *
         * @param addr Limit the search to only the devices satisfying the given partial address. Use nullptr to search
         *             for all compatible devices.
         * @return List of discovered addresses.
         */
        virtual device_list discover() = 0;

        /**
         * <p>Search for connected compatible devices.</p>
         *
         * @param addr Limit the search to only the devices satisfying the given partial address. Use nullptr to search
         *             for all compatible devices.
         * @return List of discovered addresses.
         */
        virtual device_list discover(const Address &addr) = 0;
    };
}

#endif // GENERIC_CONNECTION_HPP
