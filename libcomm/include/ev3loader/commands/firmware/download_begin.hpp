//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_FW_DOWNLOAD_BEGIN_HPP
#define EV3COMM_FW_DOWNLOAD_BEGIN_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace firmware {
            class download_begin : public command<reply_simple> {
            public:
                download_begin(packet_layer &conn, uint32_t address, uint32_t length, timeout_point due_by);

            protected:
                reply_simple run() override;

            private:
                uint32_t m_address;
                uint32_t m_length;
            };
        }
    }
}

#endif //EV3COMM_FW_DOWNLOAD_BEGIN_HPP
