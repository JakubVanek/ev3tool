//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_FW_DOWNLOAD_CONTINUE_HPP
#define EV3COMM_FW_DOWNLOAD_CONTINUE_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace firmware {
            class download_continue : public command<reply_simple> {
            public:
                download_continue(packet_layer &conn, buffer_view<> data, timeout_point due_by);

            protected:
                reply_simple run() override;

            private:
                buffer_view<> m_data;
            };
        }
    }
}

#endif //EV3COMM_FW_DOWNLOAD_CONTINUE_HPP
