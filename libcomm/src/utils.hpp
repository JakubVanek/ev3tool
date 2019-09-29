/**
 * EV3loader - the EV3 uploader
 *
 * Copyright (C) 2017  Faculty of Electrical Engineering, CTU in Prague
 * Author: Jakub Vanek <nxc4ev3@vankovi.net>
 *
 *
 * Buffer helpers.
 */

#ifndef LIBEV3COMM_UTILS_HPP
#define LIBEV3COMM_UTILS_HPP

#include <string>
#include <cstring>
#include <cstdint>
#include "buffer.hpp"

namespace ev3loader {

    /**
     * Get packet size field from the packet header.
     *
     * @param buffer Buffer containing at least 4-byte packet header.
     * @return Packet size field incremented by two.
     */
    template<typename Iterator>
    psize_t msg_size(Iterator begin, Iterator end) {
        if (end - begin < 2)
            return 0;
        uint16_t lsb = *(begin + 0);
        uint16_t msb = *(begin + 1);
        return static_cast<psize_t>((lsb | msb << 8) + 2);
    }

    /**
     * Get packet size field from the packet header.
     *
     * @param buffer Buffer containing at least 4-byte packet header.
     * @return Packet size field incremented by two.
     */
    inline psize_t msg_size(const bytearray &buffer) {
        return msg_size(buffer.begin(), buffer.end());
    }



    /**
     * Set packet size field in the packet header.
     *
     * @param buffer Buffer containing at least 4-byte packet header.
     * @param size   Packet size field (later decremented by two).
     */
    inline void msg_size(bytearray &buffer, psize_t size) {
        size -= 2;
        buffer[0] = static_cast<uint8_t>(size >> 0);
        buffer[1] = static_cast<uint8_t>(size >> 8);
    }

    /**
     * Get packet identifier field from the packet header
     *
     * @param buffer Buffer containing at least 4-byte packet header.
     * @return Packet ID field.
     */
    inline pcnt_t msg_id(const bytearray &buffer) {
        uint16_t lsb = buffer[2];
        uint16_t msb = buffer[3];
        return static_cast<pcnt_t>(lsb | msb << 8);
    }

    /**
     * Get packet identifier field from the packet header
     *
     * @param buffer Buffer containing at least 4-byte packet header.
     * @return Packet ID field.
     */
    inline pcnt_t msg_id(const transaction &tsx) {
        return msg_id(tsx.buffer);
    }

    /**
     * Set packet identifier field in the packet header
     *
     * @param buffer  Buffer containing at least 4-byte packet header.
     * @param counter Packet ID field.
     */
    inline void msg_id(bytearray &buffer, pcnt_t counter) {
        buffer[2] = static_cast<uint8_t>(counter >> 0);
        buffer[3] = static_cast<uint8_t>(counter >> 8);
    }

    /**
     * Set packet identifier field in the packet header
     *
     * @param buffer  Buffer containing at least 4-byte packet header.
     * @param counter Packet ID field.
     */
    inline void msg_id(transaction &tsx, pcnt_t counter) {
        msg_id(tsx.buffer, counter);
    }

    std::wstring narrow2wide(std::string str);

    std::string wide2narrow(std::wstring str);

    bool starts_with(const std::string &string, size_t pos, std::string &&prefix);

    void tolower_inplace(std::string &str);

    std::string tolower_copy(std::string str);
};

#endif //LIBEV3COMM_UTILS_HPP
