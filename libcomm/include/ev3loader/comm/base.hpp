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
#include <chrono>
#include <ev3loader/exceptions.hpp>
#include <ev3loader/common.hpp>
#include <ev3loader/utils/buffer.hpp>

namespace ev3loader {
    namespace comm {
        using utils::nativewindow;
        using std::chrono::milliseconds;

        using reply_future   = std::future<bytearray>;
        using reply_promise  = std::promise<bytearray>;
        using reply_callback = std::function<void(bytearray)>;
        using reply_error    = std::function<void(std::exception_ptr)>;
        using timeout_point = std::chrono::steady_clock::time_point;


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
            virtual void write(nativewindow &in,
                               milliseconds timeout,
                               bool blocking) = 0;

            /**
             * Raw synchronous read.
             *
             * @param buffer Receive buffer.
             * @param length Maximum byte number.
             * @return Actual number of bytes that were read.
             */
            virtual void read(nativewindow &out,
                              milliseconds timeout,
                              bool blocking) = 0;
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

            virtual bytearray exchange(bytearray request, timeout_point due_by) = 0;

            virtual void write_only(bytearray request) = 0;

            virtual bytearray read_only(timeout_point due_by) = 0;
        };
    }
}

#endif // GENERIC_CONNECTION_HPP
