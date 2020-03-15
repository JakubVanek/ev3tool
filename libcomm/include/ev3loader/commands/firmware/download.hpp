//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_FW_DOWNLOAD_HPP
#define EV3COMM_FW_DOWNLOAD_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace firmware {
            class download : public command<reply_simple> {
            public:
                download(packet_layer &conn, uint32_t address, bytearray data, timeout_point due_by);

            protected:
                reply_simple run() override;

            private:
                uint32_t m_address;
                bytearray       m_buffer;
                buffer_view<>   m_window;
            };
        }
    }
}

#endif //EV3COMM_FW_DOWNLOAD_HPP
