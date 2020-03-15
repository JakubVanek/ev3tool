//
// Created by kuba on 22.7.18.
//

#include <algorithm>
#include <iomanip>
#include <ev3loader/utils/bytereader.hpp>
#include <utility>

using namespace ev3loader;
using namespace ev3loader::utils;

bytereader::bytereader(bytearray data)
        : m_buffer(std::move(data)), m_window(m_buffer) {}

bytereader::bytereader(buffer_view<> view)
        : m_buffer(), m_window(view) {}

size_t bytereader::size() const {
    return static_cast<size_t>(m_window.end - m_window.begin);
}

size_t bytereader::consumed() const {
    return m_window.consumed();
}

uint8_t bytereader::take_uint8() {
    assert_available(1);
    uint8_t u8 = *(m_window.iter++);
    return u8;
}

uint16_t bytereader::take_uint16() {
    assert_available(2);
    uint16_t u16 = 0;
    u16 = static_cast<uint16_t>(*(m_window.iter++)) << 0;
    u16 |= static_cast<uint16_t>(*(m_window.iter++)) << 8;
    return u16;
}

uint32_t bytereader::take_uint32() {
    assert_available(4);
    uint32_t u32 = 0;
    u32 = static_cast<uint32_t>(*(m_window.iter++)) << 0;
    u32 |= static_cast<uint32_t>(*(m_window.iter++)) << 8;
    u32 |= static_cast<uint32_t>(*(m_window.iter++)) << 16;
    u32 |= static_cast<uint32_t>(*(m_window.iter++)) << 24;
    return u32;
}

int8_t bytereader::take_int8() {
    uint8_t temp = take_uint8();
    return reinterpret_cast<int8_t &>(temp);
}

int16_t bytereader::take_int16() {
    uint16_t temp = take_uint16();
    return reinterpret_cast<int16_t &>(temp);
}

int32_t bytereader::take_int32() {
    uint32_t temp = take_uint32();
    return reinterpret_cast<int32_t &&>(temp);
}

std::string bytereader::take_string() {
    auto end = std::find(m_window.iter, m_window.end, '\0');

    std::string str(m_window.iter, end);
    if (end == m_window.end) {
        m_window.iter = m_window.end;
    } else {
        m_window.iter = end + 1;
    }
    return str;
}

std::string bytereader::take_string_block(size_t advance) {
    assert_available(advance);
    auto end = std::find(m_window.iter, m_window.end, '\0');
    std::string str(m_window.iter, end);
    m_window.iter += advance;
    return str;
}

system_commands bytereader::take_command() {
    return static_cast<system_commands>(take_uint8());
}

system_status bytereader::take_status() {
    return static_cast<system_status>(take_uint8());
}

message_type bytereader::take_type() {
    return static_cast<message_type>(take_uint8());
}

bytearray bytereader::take_buffer(size_t size) {
    assert_available(size);
    bytearray bound(size, 0);
    buffer_view<> bound_wrap{bound};
    m_window.copy_out(bound_wrap);
    return bound;
}

bytearray bytereader::take_buffer() {
    bytearray rest(m_window.iter, m_window.end);
    m_window.iter = m_window.end;
    return rest;
}

uint16_t bytereader::take_size() {
    return take_uint16();
}

uint16_t bytereader::take_id() {
    return take_uint16();
}

file_handle bytereader::take_handle() {
    return take_uint8();
}

void bytereader::close() const {
    if (size() != consumed()) {
        throw parse_failed(parse_error::REPLY_TOO_LARGE);
    }
}

void bytereader::assert_available(size_t bytes) const {
    if ((size() - consumed()) < bytes) {
        throw parse_failed(parse_error::REPLY_TOO_SMALL);
    }
}

reply_system_header::reply_system_header(bytereader &in) {
    m_size = in.take_size();
    m_id = in.take_id();
    m_type = in.take_type();
    m_command = in.take_command();
    m_status = in.take_status();
}