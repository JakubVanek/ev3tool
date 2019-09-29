//
// Created by kuba on 22.7.18.
//

#ifndef EV3COMM_BYTEWRITER_HPP
#define EV3COMM_BYTEWRITER_HPP

#include "common.hpp"
#include "ev3proto.hpp"
#include "buffer.hpp"
#include "as_int.hpp"
#include <string>

namespace ev3loader {
    class bytewriter {
    public:
        bytewriter();

        void reset();
        bytewriter &operator<<(as_uint8 &&u8);
        bytewriter &operator<<(as_uint16 &&u16);
        bytewriter &operator<<(as_uint32 &&u32);
        bytewriter &operator<<(as_int8 &&s8);
        bytewriter &operator<<(as_int16 &&s16);
        bytewriter &operator<<(as_int32 &&s32);
        bytewriter &operator<<(as_string &&str);
        bytewriter &operator<<(as_bytes &&bytes);
        bytewriter &operator<<(as_custom &&custom);
        bytearray &buffer();
        const bytearray &buffer() const;
        size_t size() const;
        void finalize();

    private:
        bytearray                            m_buffer;
        std::back_insert_iterator<bytearray> m_iterator;
    };
}

#endif //EV3COMM_BYTEWRITER_HPP
