//
// Created by kuba on 15.01.20.
//

#ifndef EV3COMM_CONNECTION_EV3_HPP
#define EV3COMM_CONNECTION_EV3_HPP

#include <ev3loader/comm/base.hpp>
#include <queue>

namespace ev3loader {
    namespace comm {
        class ev3_connection : public packet_layer {
        public:
            explicit
            ev3_connection(std::unique_ptr<channel> link);

            bytearray exchange(bytearray request, timeout_point due_by) override;

            void write_only(bytearray request) override;

            bytearray read_only(timeout_point due_by) override;

        private:
            pcnt_t do_push(bytearray request);
            bytearray do_receive(timeout_point due_by);

            pcnt_t prepare_for_send(bytearray &request);

            milliseconds prepare_timeout(timeout_point due_by) const;
            std::unique_ptr<channel> m_link;
            std::mutex m_lock;
            pcnt_t m_msgid;
        };
    }
}

#endif //EV3COMM_CONNECTION_EV3_HPP
