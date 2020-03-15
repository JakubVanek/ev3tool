//
// Created by kuba on 22.7.18.
//

#ifndef EV3COMM_BYTEREADER_HPP
#define EV3COMM_BYTEREADER_HPP

#include "ev3loader/common.hpp"
#include "buffer.hpp"
#include "ev3loader/ev3proto.hpp"
#include "ev3loader/exceptions.hpp"

namespace ev3loader {
    namespace utils {

        class reply_system_header;

        class bytereader {
        public:
            explicit
            bytereader(bytearray data);

            explicit
            bytereader(buffer_view<bytearray> view);

            template<typename T>
            T take() {
                return T{*this};
            }

            int8_t take_int8();

            int16_t take_int16();

            int32_t take_int32();

            uint8_t take_uint8();

            uint16_t take_uint16();

            uint32_t take_uint32();

            std::string take_string_block(size_t advance);

            bytearray take_buffer(size_t size);

            std::string take_string();

            bytearray take_buffer();

            uint16_t take_size();

            uint16_t take_id();

            file_handle take_handle();

            system_commands take_command();

            system_status take_status();

            message_type take_type();

            void close() const;


            size_t size() const;

            size_t consumed() const;

        private:
            void assert_available(size_t bytes) const;

            bytearray m_buffer;
            buffer_view<bytearray> m_window;
        };

        class reply_system_header {
        public:
            explicit
            reply_system_header(bytereader &in);

            psize_t size() const {
                return m_size;
            }

            pcnt_t id() const {
                return m_id;
            }

            message_type type() const {
                return m_type;
            }

            system_commands command() const {
                return m_command;
            }

            system_status result() const {
                return m_status;
            }

            reply_system_header &verify(system_commands sent) {
                assert_throw<sys_remote_error>(m_type == SYS_RECV_REPLY, m_command, m_status);
                assert_throw<parse_failed>(m_command == sent, REPLY_CMD_MISMATCH);
                return *this;
            }

        private:
            psize_t m_size;
            pcnt_t m_id;
            message_type m_type;
            system_commands m_command;
            system_status m_status;
        };
    }
}

#endif //EV3COMM_BYTEREADER_HPP
