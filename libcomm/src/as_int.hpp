//
// Created by kuba on 22.7.18.
//

#ifndef EV3COMM_AS_INT_HPP
#define EV3COMM_AS_INT_HPP

#include <cstdint>
#include "ev3proto.hpp"
#include "buffer.hpp"

namespace ev3loader {
    class bytewriter;

    template<typename T>
    class as_impl {
    public:
        typedef T type;

        template<typename U>
        explicit
        as_impl(U data) : m_stor(static_cast<T>(data)) {}

        explicit
        operator type() {
            return m_stor;
        }

        const type &get() const {
            return m_stor;
        }

    protected:
        type m_stor;
    };

    class as_int8 : public as_impl<int8_t> {
        using as_impl::as_impl;
    };

    class as_int16 : public as_impl<int16_t> {
        using as_impl::as_impl;
    };

    class as_int32 : public as_impl<int32_t> {
        using as_impl::as_impl;
    };

    class as_int64 : public as_impl<int64_t> {
        using as_impl::as_impl;
    };

    class as_uint8 : public as_impl<uint8_t> {
        using as_impl::as_impl;
    };

    class as_uint16 : public as_impl<uint16_t> {
        using as_impl::as_impl;
    };

    class as_uint32 : public as_impl<uint32_t> {
        using as_impl::as_impl;
    };

    class as_uint64 : public as_impl<uint64_t> {
        using as_impl::as_impl;
    };

    class as_string : public as_impl<std::string> {
        using as_impl::as_impl;
    };


    class as_bytes {
    public:
        typedef buffer_view<> type;

        explicit
        as_bytes(type data) : m_stor(data) {}

        explicit
        as_bytes(type::buffer_type data) : m_stor(data) {}

        explicit
        operator type() {
            return m_stor;
        }

        const type &get() const {
            return m_stor;
        }

    protected:
        type m_stor;
    };

    class as_custom{
    public:
        as_custom() = default;
        virtual ~as_custom() = default;
        virtual void doIt(bytewriter &out) const = 0;
    };

    class as_system_header : public as_custom {
    public:
        explicit
        as_system_header(message_type type, system_commands cmd);
        ~as_system_header() override;
        void doIt(bytewriter &out) const override;
    private:
        message_type m_type;
        system_commands m_command;
    };

    typedef as_uint16 as_size;
    typedef as_uint16 as_id;
    typedef as_uint8  as_command;
    typedef as_uint8  as_type;
    typedef as_uint8  as_status;
    typedef as_uint8  as_handle;
}

#endif //EV3COMM_AS_INT_HPP
