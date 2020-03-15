//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_FWUPDATE_HPP
#define EV3COMM_FWUPDATE_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {

            class enter_fwupdate : public command<void> {
            public:
                explicit enter_fwupdate(packet_layer &conn) : command(conn) {}

            protected:
                void run() override;
            };
        }
    }
}

#endif //EV3COMM_FWUPDATE_HPP
