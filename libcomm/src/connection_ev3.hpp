//
// Created by kuba on 30.9.17.
//

#ifndef EV3_CONNECTION_HPP
#define EV3_CONNECTION_HPP

#include <map>
#include <shared_mutex>
#include <queue>
#include <condition_variable>
#include <mutex>
#include "connection.hpp"
#include "threading.hpp"

namespace ev3loader {
    class rxqueue;
    class txqueue;
    class bookkeeper;

    class bookkeeper {
    public:
        bookkeeper(txqueue &tx);

        pcnt_t register_transaction(transaction &tsx);

        void trigger(pcnt_t id);

        void finish(const bytearray &pkt);

        void fail(packet_callback::failure_type ret);

        void set_default_callback(packet_callback call);

        void mark_sent(pcnt_t id);

        bool has_unfinished_read();
    private:
        struct record {
            using clock      = std::chrono::steady_clock;
            using duration   = clock::duration;
            using time_point = clock::time_point;

            record();

            explicit record(const transaction &tsx);

            void trigger();

            bool timed_out() const;

            void mark_sent();

            bool was_sent() const;

            packet_callback retfn;
            time_point      time_last;
            duration        timeout;
            bool            sent;
        };

        using map_type = std::map<pcnt_t, record>;

        void check_timeouts();
        packet_callback get_call(pcnt_t id);

        txqueue                *m_txqueue;
        packet_callback         m_retfn_dfl;
        map_type                m_bookings;
        pcnt_t                  m_counter;
        std::mutex    m_maplock;
        std::mutex              m_sleeplock;
        std::condition_variable m_sleeper;
        loop_thread             m_thr;
    };

    class rxqueue {
    public:
        rxqueue(bookkeeper &reg, channel &link);

        ~rxqueue();

    private:
        void dowork();

        void push(buffer_view<> input);

        bytearray     m_rxbuf;
        nativewindow  m_rxwin;
        bytearray     m_rasbuf;
        buffer_view<> m_raswin;

        bookkeeper  *m_reg;
        channel     *m_link;
        loop_thread m_thr;
    };

    class txqueue {
    public:
        txqueue(bookkeeper &reg, channel &link);

        ~txqueue();

        void push(bytearray pkt);

        void wake_on_rx();
    private:
        struct task {
            task();

            explicit
            task(const bytearray &sndbuf);

            bytearray    m_sndbuf;
            nativewindow m_sndwindow;
        };

        void dowork();

        task *current();

        void pop();

        std::mutex       m_sendlock;
        std::queue<task> m_sendqueue;

        std::condition_variable m_notify;
        bookkeeper              *m_reg;
        channel                 *m_link;

        loop_thread m_thr;
    };

    /**
     * EV3 HIDAPI packet connection.
     */
    class ev3_connection final : public packet_layer {
    public:
        ev3_connection() = delete;

        /**
         * Allocate connection to a device.
         * @param link Device channel.
         */
        explicit ev3_connection(std::unique_ptr<channel> link);

        ~ev3_connection() override;

        ev3_connection(const ev3_connection &other) = delete;

        ev3_connection(ev3_connection &&other) = delete;

        ev3_connection &operator=(const ev3_connection &other) = delete;

        ev3_connection &operator=(ev3_connection &&other) = delete;

        // INHERITED

        pcnt_t push_packet(transaction &pkt) override;

        void default_callback(packet_callback on_receive) override;

    private:
        /**
         * USB link data.
         */
        std::unique_ptr<channel> m_conn;
        bookkeeper               m_reg;
        txqueue                  m_output;
        rxqueue                  m_input;
    };
}

#endif //EV3_CONNECTION_HPP
