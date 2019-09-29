//
// Created by kuba on 23.7.18.
//

#ifndef EV3COMM_COMMANDS_DIRECT_HPP
#define EV3COMM_COMMANDS_DIRECT_HPP

#include "commands.hpp"

namespace ev3loader {
    namespace commands {
        enum class direct_command_type {
            WITH_REPLY, WITHOUT_REPLY
        };
        enum class direct_command_result {
            SUCCESS, FAILURE
        };
        struct request_direct {
            buffer_view<>       bytecode;
            direct_command_type type;
            uint16_t            globals;
            uint8_t             locals;
        };
        struct reply_direct {
            bytearray             globals;
            direct_command_result result;
        };

        class direct_command : public data_command<direct_command, request_direct, reply_direct> {
        public:
            direct_command(packet_layer &conn, input_type const &in);

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };
    }
}

#endif //EV3COMM_COMMANDS_DIRECT_HPP
