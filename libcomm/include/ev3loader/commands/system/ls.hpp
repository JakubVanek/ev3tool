//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_LS_HPP
#define EV3COMM_LS_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
            enum class file_type {
                    file,
                    directory
            };

            struct dir_entry {
                dir_entry() = default;

                std::string name{};
                file_type what = file_type::file;
                size_t size = 0;
                std::array<uint8_t, 16> md5{};
            };

            struct reply_ls {
                std::vector<dir_entry> listing;
                system_status result = system_status::UNKNOWN_ERROR;
            };

            class ls : public command<reply_ls> {
            public:
                ls(packet_layer &conn, std::string path, timeout_point due_by);

            protected:
                output_type run() override;

            private:
                std::string m_path;
            };
        }
    }
}

#endif //EV3COMM_LS_HPP
