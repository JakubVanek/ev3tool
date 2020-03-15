//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_DOWNLOAD_CONTINUE_HPP
#define EV3COMM_DOWNLOAD_CONTINUE_HPP

#include <ev3loader/commands/base.hpp>
#include <ev3loader/commands/system/download_common.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
            class download_continue : public command<reply_download> {
            public:
                download_continue(packet_layer &conn,
                                  file_handle handle,
                                  buffer_view<> segment,
                                  timeout_point due_by);

            protected:
                output_type run() override;

            private:
                file_handle m_handle;
                buffer_view<> m_segment;
            };
        }
    }
}

#endif //EV3COMM_DOWNLOAD_CONTINUE_HPP
