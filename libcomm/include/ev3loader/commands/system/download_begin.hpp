//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_DOWNLOAD_BEGIN_HPP
#define EV3COMM_DOWNLOAD_BEGIN_HPP

#include <ev3loader/commands/base.hpp>
#include <ev3loader/commands/system/download_common.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
            class download_begin : public command<reply_download> {
            public:
                download_begin(packet_layer &conn,
                               std::string path,
                               size_t file_length,
                               timeout_point due_by);

            protected:
                output_type run() override;

            private:
                std::string m_path;
                size_t m_file_size;
            };
        }
    }
}

#endif //EV3COMM_DOWNLOAD_BEGIN_HPP
