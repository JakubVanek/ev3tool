//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_UPLOAD_HPP
#define EV3COMM_UPLOAD_HPP

#include <ev3loader/commands/base.hpp>
#include <ev3loader/commands/system/upload_common.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
            class upload : public command<reply_upload> {
            public:
                upload(packet_layer &conn,
                       std::string path,
                       upload_type which,
                       timeout_point due_by);

            protected:
                output_type run() override;

            private:
                reply_upload send_first();

                reply_upload send_next();

                bool process(reply_upload &&reply);

                size_t handle_new_size(reply_upload &reply);

                bool decide_on_exitcode(system_status reply);

                std::string m_path;
                upload_type m_op;
                buffer_view<> m_window;
                reply_upload m_whole;
                system_commands m_last_command;
            };
        }
    }
}

#endif //EV3COMM_UPLOAD_HPP
