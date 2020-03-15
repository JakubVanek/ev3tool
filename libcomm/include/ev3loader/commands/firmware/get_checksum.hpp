//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_GET_CHECKSUM_HPP
#define EV3COMM_GET_CHECKSUM_HPP

#include <ev3loader/commands/base.hpp>

namespace ev3loader {
    namespace commands {
        namespace firmware {
            typedef uint32_t crc32_t;

            class get_checksum : public command<crc32_t> {
            public:
                get_checksum(packet_layer &conn, uint32_t address, uint32_t length, timeout_point due_by);

            protected:
                crc32_t run() override;

            private:
                uint32_t m_address;
                uint32_t m_length;
            };
        }
    }
}

#endif //EV3COMM_GET_CHECKSUM_HPP
