//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_WRITE_MBOX_HPP
#define EV3COMM_WRITE_MBOX_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace system {
            class write_mbox : public command<void> {
            public:
                write_mbox(packet_layer &conn, std::string name, bytearray data, timeout_point due_by);

            protected:
                void run() override;

            private:
                std::string m_name;
                bytearray m_data;
            };
        }
    }
}

#endif //EV3COMM_WRITE_MBOX_HPP
