//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_UPLOAD_BEGIN_HPP
#define EV3COMM_UPLOAD_BEGIN_HPP

#include <ev3loader/commands/base.hpp>
#include <ev3loader/commands/system/upload_common.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
            class upload_begin : public command<reply_upload> {
            public:
                upload_begin(packet_layer &conn,
                             std::string path,
                             size_t nextSize,
                             upload_type type,
                             timeout_point due_by);

                system_commands get_command() const {
                    return m_command;
                }

            protected:
                output_type run() override;

            private:
                std::string m_path;
                size_t m_next_request;
                system_commands m_command;
            };
        }
    }
}

#endif //EV3COMM_UPLOAD_BEGIN_HPP
