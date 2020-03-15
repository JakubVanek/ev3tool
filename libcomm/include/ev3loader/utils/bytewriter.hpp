//
// Created by kuba on 22.7.18.
//

#ifndef EV3COMM_BYTEWRITER_HPP
#define EV3COMM_BYTEWRITER_HPP

#include <iterator>
#include <type_traits>
#include "ev3loader/common.hpp"
#include "buffer.hpp"
#include "ev3loader/ev3proto.hpp"
#include "ev3loader/config.hpp"

namespace ev3loader {
    namespace utils {
        class bytewriter {
        public:
            typedef std::back_insert_iterator<bytearray> inserter;

            bytewriter();

            template<typename T>
            bytewriter &operator<<(T &&operand) {
                operand.insert(m_iterator, *this);
                return *this;
            }

            size_t size() const;

            bytearray finalize();

        private:
            bytearray m_buffer;
            inserter m_iterator;
        };

        template<typename T>
        struct as_base {
            using inner_type = T;

            explicit
            as_base(T data)
                    : m_inner(std::move(data)) {}

        protected:
            inner_type m_inner;
        };

        struct as_int8 : as_base<int8_t> {
            using as_base<int8_t>::as_base;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        struct as_int16 : as_base<int16_t> {
            using as_base<int16_t>::as_base;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        struct as_int32 : as_base<int32_t> {
            using as_base<int32_t>::as_base;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        struct as_int64 : as_base<int64_t> {
            using as_base<int64_t>::as_base;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        struct as_uint8 : as_base<uint8_t> {
            using as_base<uint8_t>::as_base;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        struct as_uint16 : as_base<uint16_t> {
            using as_base<uint16_t>::as_base;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        struct as_uint32 : as_base<uint32_t> {
            using as_base<uint32_t>::as_base;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        struct as_uint64 : as_base<uint64_t> {
            using as_base<uint64_t>::as_base;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        struct as_string : as_base<std::string> {
            using as_base<std::string>::as_base;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };


        struct as_bytes {
        public:
            using inner_type = buffer_view<>;

            explicit
            as_bytes(inner_type::buffer_type &data)
                    : m_view(data) {}

            explicit
            as_bytes(inner_type view)
                    : m_view(view) {}

            void insert(bytewriter::inserter &next, bytewriter &raw);

        protected:
            inner_type m_view;
        };

        class as_system_header {
        public:
            explicit
            as_system_header(message_type type, system_commands cmd);

            void insert(bytewriter::inserter &next, bytewriter &raw);

        private:
            message_type m_type;
            system_commands m_command;
        };

        typedef as_string as_null_terminated_string;

        struct as_u8len_null_terminated_string : public as_string {
            using as_string::as_string;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        struct as_u16len_bytes : public as_bytes {
            using as_bytes::as_bytes;

            void insert(bytewriter::inserter &next, bytewriter &raw);
        };

        typedef as_uint16 as_size;
        typedef as_uint16 as_id;
        typedef as_uint8 as_command;
        typedef as_uint8 as_type;
        typedef as_uint8 as_status;
        typedef as_uint8 as_handle;

        struct close_small {
            void insert(bytewriter::inserter &next, bytewriter &out) {
                if (out.size() > FILE_BUF_SIZE) {
                    throw std::out_of_range("Message is too big for a single transfer");
                }
            }
        };

        struct close_big {
            void insert(bytewriter::inserter &next, bytewriter &out) {
                if (out.size() > UINT16_MAX) {
                    throw std::out_of_range("Message is too big for a single transaction");
                }
            }
        };
    }
}

#endif //EV3COMM_BYTEWRITER_HPP
