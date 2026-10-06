// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/stdint.hpp>

namespace websocketpp {
namespace lib {
namespace net {

inline bool is_little_endian() {
    short int val = 0x1;
    char *ptr = reinterpret_cast<char *>(&val);
    return (ptr[0] == 1);
}

/// Convert a 16-bit value from host to network byte order without socket APIs.
inline uint16_t _htons(uint16_t src) {
    if (!is_little_endian()) {
        return src;
    }
    return static_cast<uint16_t>((src >> 8) | (src << 8));
}

/// Convert a 16-bit value from network to host byte order.
inline uint16_t _ntohs(uint16_t src) {
    return _htons(src);
}

/// Convert a 32-bit value from host to network byte order without socket APIs.
inline uint32_t _htonl(uint32_t src) {
    if (!is_little_endian()) {
        return src;
    }
    return ((src & 0x000000ffU) << 24) |
           ((src & 0x0000ff00U) << 8) |
           ((src & 0x00ff0000U) >> 8) |
           ((src & 0xff000000U) >> 24);
}

/// Convert a 32-bit value from network to host byte order.
inline uint32_t _ntohl(uint32_t src) {
    return _htonl(src);
}

#define TYP_INIT 0
#define TYP_SMLE 1
#define TYP_BIGE 2

/// Convert 64 bit value to network byte order
/**
 * This method is prefixed to avoid conflicts with operating system level
 * macros for this functionality.
 *
 * TODO: figure out if it would be beneficial to use operating system level
 * macros for this.
 *
 * @param src The integer in host byte order
 * @return src converted to network byte order
 */
inline uint64_t _htonll(uint64_t src) {
    // C++11 synchronizes local-static initialization across threads.
    static const int typ = is_little_endian() ? TYP_SMLE : TYP_BIGE;
    unsigned char c;
    union {
        uint64_t ull;
        unsigned char c[8];
    } x;
    if (typ == TYP_BIGE)
        return src;
    x.ull = src;
    c = x.c[0]; x.c[0] = x.c[7]; x.c[7] = c;
    c = x.c[1]; x.c[1] = x.c[6]; x.c[6] = c;
    c = x.c[2]; x.c[2] = x.c[5]; x.c[5] = c;
    c = x.c[3]; x.c[3] = x.c[4]; x.c[4] = c;
    return x.ull;
}

/// Convert 64 bit value to host byte order
/**
 * This method is prefixed to avoid conflicts with operating system level
 * macros for this functionality.
 *
 * TODO: figure out if it would be beneficial to use operating system level
 * macros for this.
 *
 * @param src The integer in network byte order
 * @return src converted to host byte order
 */
inline uint64_t _ntohll(uint64_t src) {
    return _htonll(src);
}

} // net
} // lib
} // websocketpp
