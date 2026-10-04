// Copyright (c) 2026, Green Lightning
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace websocketpp {
namespace sha1 {

using digest = std::array<std::uint8_t, 20>;
namespace detail {

inline std::uint32_t rotate_left(std::uint32_t value, unsigned shift) {
    return (value << shift) | (value >> (32 - shift));
}

inline void compress(unsigned char const* block, std::uint32_t (&state)[5]) {
    std::uint32_t words[80];
    for (std::size_t i = 0; i < 16; ++i) {
        words[i] = (std::uint32_t(block[4 * i]) << 24)
                 | (std::uint32_t(block[4 * i + 1]) << 16)
                 | (std::uint32_t(block[4 * i + 2]) << 8)
                 | std::uint32_t(block[4 * i + 3]);
    }
    for (std::size_t i = 16; i < 80; ++i) {
        words[i] = rotate_left(words[i - 3] ^ words[i - 8]
                            ^ words[i - 14] ^ words[i - 16], 1);
    }

    std::uint32_t a = state[0], b = state[1], c = state[2];
    std::uint32_t d = state[3], e = state[4];
    for (std::size_t i = 0; i < 80; ++i) {
        std::uint32_t f, k;
        if (i < 20) {
            f = (b & c) | (~b & d);
            k = 0x5a827999u;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ed9eba1u;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8f1bbcdcu;
        } else {
            f = b ^ c ^ d;
            k = 0xca62c1d6u;
        }
        const std::uint32_t next = rotate_left(a, 5) + f + e + k + words[i];
        e = d;
        d = c;
        c = rotate_left(b, 30);
        b = a;
        a = next;
    }
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

} // namespace detail

// data may be null only when size is zero.
inline digest calc(void const* data, std::size_t size) {
    if (data == nullptr && size != 0) {
        throw std::invalid_argument("sha1::calc: null data with nonzero size");
    }
    std::uint32_t state[5] = {
        0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u, 0xc3d2e1f0u
    };
    const std::uint64_t bit_length = static_cast<std::uint64_t>(size) * 8;
    auto bytes = static_cast<unsigned char const*>(data);
    while (size >= 64) {
        detail::compress(bytes, state);
        bytes += 64;
        size -= 64;
    }

    unsigned char tail[128] = {};
    if (size != 0) {
        std::memcpy(tail, bytes, size);
    }
    tail[size] = 0x80;
    const std::size_t padded_size = size < 56 ? 64 : 128;
    for (std::size_t i = 0; i < 8; ++i) {
        tail[padded_size - 1 - i] = static_cast<unsigned char>(bit_length >> (8 * i));
    }
    detail::compress(tail, state);
    if (padded_size == 128) {
        detail::compress(tail + 64, state);
    }

    digest result{};
    for (std::size_t i = 0; i < 5; ++i) {
        for (std::size_t j = 0; j < 4; ++j) {
            result[4 * i + j] = static_cast<std::uint8_t>(state[i] >> (24 - 8 * j));
        }
    }
    return result;
}

} // namespace sha1
} // namespace websocketpp
