//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_SET_BT_PIN_HPP
#define EV3COMM_SET_BT_PIN_HPP

#include <cstdint>
#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {

            struct reply_setpin {
                uint8_t brickMac[6];
                uint32_t brickPin;
            };

            class set_bt_pin : public command<reply_setpin> {
            public:
                set_bt_pin(packet_layer &conn, uint8_t hostAddress[6], uint32_t brickPin, timeout_point due_by);

            protected:
                output_type run() override;

            private:
                bytereader send();
                static output_type process(bytereader &&reply);

                uint8_t m_host_address[6];
                uint32_t m_brick_pin;
            };
        }
    }
}

#endif //EV3COMM_SET_BT_PIN_HPP
