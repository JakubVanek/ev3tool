//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_ERASE_HPP
#define EV3COMM_ERASE_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace firmware {
            class erase : public command<reply_simple> {
            public:
                using command::command;

            protected:
                reply_simple run() override;
            };
        }
    }
}

#endif //EV3COMM_ERASE_HPP
