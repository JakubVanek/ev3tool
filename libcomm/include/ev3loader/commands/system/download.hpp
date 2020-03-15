//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_DOWNLOAD_HPP
#define EV3COMM_DOWNLOAD_HPP

#include <ev3loader/commands/base.hpp>
#include <ev3loader/commands/system/download_common.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
            class download : public command<reply_download> {
            public:
                download(packet_layer &conn,
                         std::string path,
                         bytearray data,
                         timeout_point due_by);

            protected:
                output_type run() override;

            private:
                reply_download send_first();

                reply_download send_next();

                bool process(reply_download &reply, bool first);

                std::string m_path;
                bytearray m_data;
                buffer_view<> m_window;
                file_handle m_handle;
                reply_download m_result;
            };
        }
    }
}

#endif //EV3COMM_DOWNLOAD_HPP
