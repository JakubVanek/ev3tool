//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_FILEOP_HPP
#define EV3COMM_FILEOP_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
            // MKDIR/UNLINK
            enum class fileop_type {
                mkdir,
                unlink
            };

            class fileop_command : public command<reply_simple> {
            public:
                fileop_command(packet_layer &conn,
                               std::string path,
                               fileop_type what,
                               timeout_point due_by);

            protected:
                output_type run() override;

            private:
                system_commands m_command;
                std::string m_path;
            };

            struct mkdir : fileop_command {
                mkdir(packet_layer &conn, std::string path, timeout_point due_by)
                        : fileop_command(conn, std::move(path), fileop_type::mkdir, due_by) {}
            };

            struct unlink : fileop_command {
                unlink(packet_layer &conn, std::string path, timeout_point due_by)
                        : fileop_command(conn, std::move(path), fileop_type::unlink, due_by) {}
            };
        }
    }
}

#endif //EV3COMM_FILEOP_HPP
