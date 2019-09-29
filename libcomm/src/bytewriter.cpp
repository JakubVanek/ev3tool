//
// Created by kuba on 22.7.18.
//

#include <limits>
#include <cstring>
#include "bytewriter.hpp"
#include "utils.hpp"

namespace ev3loader {
    template<typename To, typename From>
    To sign_cast(const From &number) {
        return To(reinterpret_cast<const typename To::type&>(number.get()));
    };

    bytewriter::bytewriter() : m_buffer(), m_iterator(std::back_inserter(m_buffer)) {
    }

    void bytewriter::reset() {
        m_buffer.clear();
        m_iterator = std::back_inserter(m_buffer);
    }

    bytewriter &bytewriter::operator<<(as_uint8 && u8) {
        *m_iterator = u8.get();
        return *this;
    }

    bytewriter &bytewriter::operator<<(as_uint16 && u16) {
        *m_iterator = static_cast<uint8_t>(u16.get() >> 0);
        *m_iterator = static_cast<uint8_t>(u16.get() >> 8);
        return *this;
    }

    bytewriter &bytewriter::operator<<(as_uint32 && u32) {
        *m_iterator = static_cast<uint8_t>(u32.get() >> 0);
        *m_iterator = static_cast<uint8_t>(u32.get() >> 8);
        *m_iterator = static_cast<uint8_t>(u32.get() >> 16);
        *m_iterator = static_cast<uint8_t>(u32.get() >> 24);
        return *this;
    }

    bytewriter &bytewriter::operator<<(as_int8 && s8) {
        return operator<<(sign_cast<as_uint8>(s8));
    }

    bytewriter &bytewriter::operator<<(as_int16 && s16) {
        return operator<<(sign_cast<as_uint16>(s16));
    }

    bytewriter &bytewriter::operator<<(as_int32 && s32) {
        return operator<<(sign_cast<as_uint32>(s32));
    }

    bytewriter &bytewriter::operator<<(as_string &&asciiz) {
        std::copy(asciiz.get().begin(), asciiz.get().end(), m_iterator);
        *m_iterator = '\0';
        return *this;
    }

    bytearray &bytewriter::buffer() {
        finalize();
        return m_buffer;
    }

    const bytearray &bytewriter::buffer() const {
        return m_buffer;
    }

    bytewriter &bytewriter::operator<<(as_bytes &&bytes) {
        std::copy(bytes.get().iter, bytes.get().end, m_iterator);
        return *this;
    }

    size_t bytewriter::size() const {
        return m_buffer.size();
    }

    void bytewriter::finalize() {
        msg_size(m_buffer, m_buffer.size());
    }

    bytewriter &bytewriter::operator<<(as_custom &&custom) {
        custom.doIt(*this);
        return *this;
    }
}