//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_UPLOAD_CONTINUE_HPP
#define EV3COMM_UPLOAD_CONTINUE_HPP

#include <ev3loader/commands/base.hpp>
#include <ev3loader/commands/system/upload_common.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
            class upload_continue : public command<reply_upload> {
            public:
                upload_continue(packet_layer &conn,
                                file_handle handle,
                                size_t nextSize,
                                size_t total,
                                upload_type type,
                                timeout_point due_by);

                system_commands get_command() const {
                    return m_command;
                }

            protected:
                output_type run() override;

            private:
                file_handle m_handle;
                size_t m_next_request;
                size_t m_total_size;
                system_commands m_command;
            };
        }
    }
}

#endif //EV3COMM_UPLOAD_CONTINUE_HPP
