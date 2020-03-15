//
// Created by kuba on 22.6.18.
//

#ifndef LIBEV3COMM_COMMON_HPP
#define LIBEV3COMM_COMMON_HPP

#include <cstdint>
#include <vector>

namespace ev3loader {
    /**
     * <p>Packet size -- unsigned variant.</p>
     */
    typedef std::size_t psize_t;
    /**
     * <p>Packet size -- signed variant for error passing.</p>
     */
    typedef std::ptrdiff_t pssize_t;
    /**
     * <p>Packet counter -- used as transaction identifier.</p>
     */
    typedef std::uint16_t pcnt_t;
    /**
     * <p>Packet payload.</p>
     */
    typedef std::uint8_t byte;
    /**
     * <p>Managed memory.</p>
     */
    typedef std::vector<byte> bytearray;
    typedef uint8_t file_handle;
    typedef int16_t file_shandle;
}

#endif //LIBEV3COMM_COMMON_HPP
