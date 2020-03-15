//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_GET_VERSION_HPP
#define EV3COMM_GET_VERSION_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace firmware {
            struct version {
                uint32_t hardwareVersion;
                uint32_t firmwareVersion;
            };

            class get_version : public command<version> {
            public:
                using command::command;

            protected:
                version run() override;
            };
        }
    }
}

#endif //EV3COMM_GET_VERSION_HPP
