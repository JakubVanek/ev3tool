//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_CLOSEFD_HPP
#define EV3COMM_CLOSEFD_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
        class closefd : public command<reply_simple> {
            public:
                closefd(packet_layer &conn, file_handle handle, timeout_point due_by);

            protected:
                output_type run() override;

            private:
                file_handle m_handle;
            };
        }
    }
}

#endif //EV3COMM_CLOSEFD_HPP
