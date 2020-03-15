//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_EXECUTE_WITHOUT_REPLY_HPP
#define EV3COMM_EXECUTE_WITHOUT_REPLY_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace direct {
            class execute_without_reply : public command<void> {
            public:
                execute_without_reply(packet_layer &conn,
                                      buffer_view<> bytecode,
                                      uint16_t globals,
                                      uint16_t locals);

            protected:
                void run() override;

            private:
                buffer_view<> m_code;
                uint16_t m_globals;
                uint16_t m_locals;
            };
        }
    }
}

#endif //EV3COMM_EXECUTE_WITHOUT_REPLY_HPP
