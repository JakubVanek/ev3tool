//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_REBOOT_HPP
#define EV3COMM_REBOOT_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace firmware {
            class reboot : public command<reply_simple> {
            public:
                using command::command;

            protected:
                reply_simple run() override;
            };
        }
    }
}

#endif //EV3COMM_REBOOT_HPP
