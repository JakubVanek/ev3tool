#ifndef EV3COMM_EXECUTE_HPP
#define EV3COMM_EXECUTE_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace direct {
            enum class direct_command_result {
                SUCCESS, FAILURE
            };

            struct reply_direct {
                bytearray globals;
                direct_command_result result;
            };

            class execute : public command<reply_direct> {
            public:
                execute(packet_layer &conn,
                        buffer_view<> bytecode,
                        uint16_t globals,
                        uint16_t locals,
                        timeout_point due_by);

            protected:
                reply_direct run() override;

            private:
                buffer_view<> m_code;
                uint16_t m_globals;
                uint16_t m_locals;
            };

            bytewriter direct_request(message_type msg,
                                      buffer_view<> code,
                                      uint16_t globals,
                                      uint16_t locals);
        }
    }
}

#endif //EV3COMM_EXECUTE_HPP
