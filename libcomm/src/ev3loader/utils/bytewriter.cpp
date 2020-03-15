//
// Created by kuba on 22.7.18.
//

#include <ev3loader/utils/bytewriter.hpp>
#include <ev3loader/utils/utils.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;

bytewriter::bytewriter()
        : m_buffer(), m_iterator(std::back_inserter(m_buffer)) {}


size_t bytewriter::size() const {
    return m_buffer.size();
}

bytearray bytewriter::finalize() {
    return std::move(m_buffer);
}

as_system_header::as_system_header(message_type type, system_commands cmd)
        : m_type(type), m_command(cmd) {}

void as_int8::insert(bytewriter::inserter &next, bytewriter &raw) {
    return as_uint8(m_inner).insert(next, raw);
}

void as_int16::insert(bytewriter::inserter &next, bytewriter &raw) {
    return as_uint16(m_inner).insert(next, raw);
}

void as_int32::insert(bytewriter::inserter &next, bytewriter &raw) {
    return as_uint32(m_inner).insert(next, raw);
}

void as_int64::insert(bytewriter::inserter &next, bytewriter &raw) {
    return as_uint64(m_inner).insert(next, raw);
}

void as_uint8::insert(bytewriter::inserter &next, bytewriter &raw) {
    *next = m_inner;
}

void as_uint16::insert(bytewriter::inserter &next, bytewriter &raw) {
    as_uint8(static_cast<uint8_t>(m_inner >> 0u)).insert(next, raw);
    as_uint8(static_cast<uint8_t>(m_inner >> 8u)).insert(next, raw);
}

void as_uint32::insert(bytewriter::inserter &next, bytewriter &raw) {
    as_uint16(static_cast<uint16_t>(m_inner >> 0u)).insert(next, raw);
    as_uint16(static_cast<uint16_t>(m_inner >> 16u)).insert(next, raw);
}

void as_uint64::insert(bytewriter::inserter &next, bytewriter &raw) {
    as_uint32(static_cast<uint32_t>(m_inner >> 0u)).insert(next, raw);
    as_uint32(static_cast<uint32_t>(m_inner >> 32u)).insert(next, raw);
}

void as_string::insert(bytewriter::inserter &next, bytewriter &raw) {
    std::copy(m_inner.begin(), m_inner.end(), next);
    *next = '\0';
}

void as_bytes::insert(bytewriter::inserter &next, bytewriter &raw) {
    std::copy(m_view.iter, m_view.end, next);
}

void as_system_header::insert(bytewriter::inserter &next, bytewriter &raw) {
    as_size(0).insert(next, raw);
    as_id(0).insert(next, raw);
    as_type(m_type).insert(next, raw);
    as_command(m_command).insert(next, raw);
}

void as_u8len_null_terminated_string::insert(bytewriter::inserter &next, bytewriter &raw) {
    if (m_inner.size() > 0xFE)
        throw std::logic_error("Cannot fit in more than 254 characters.");

    as_uint8(m_inner.size() + 1).insert(next, raw);
    as_string::insert(next, raw);
}

void as_u16len_bytes::insert(bytewriter::inserter &next, bytewriter &raw) {
    if (m_view.available() > 0xFFFF)
        throw std::logic_error("Cannot fit in more than 64k bytes.");

    as_uint16(m_view.available()).insert(next, raw);
    as_bytes::insert(next, raw);
}