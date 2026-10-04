// Copyright (c) 2026, Green Lightning
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

namespace websocketpp {
namespace base64 {

// Standard Base64 with '=' padding. data may be null only for size zero.
inline std::string encode(void const* data, std::size_t size) {
    if (data == nullptr && size != 0) {
        throw std::invalid_argument("base64::encode: null data with nonzero size");
    }
    static char const alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    const std::size_t groups = size / 3 + (size % 3 != 0);
    if (groups > result.max_size() / 4) {
        throw std::length_error("base64::encode: output too large");
    }
    result.resize(groups * 4);
    auto bytes = static_cast<unsigned char const*>(data);
    std::size_t output = 0;
    while (size >= 3) {
        result[output++] = alphabet[bytes[0] >> 2];
        result[output++] = alphabet[((bytes[0] & 0x03) << 4) | (bytes[1] >> 4)];
        result[output++] = alphabet[((bytes[1] & 0x0f) << 2) | (bytes[2] >> 6)];
        result[output++] = alphabet[bytes[2] & 0x3f];
        bytes += 3;
        size -= 3;
    }
    if (size != 0) {
        result[output++] = alphabet[bytes[0] >> 2];
        result[output++] = alphabet[((bytes[0] & 0x03) << 4)
                                  | (size == 2 ? bytes[1] >> 4 : 0)];
        result[output++] = size == 2 ? alphabet[(bytes[1] & 0x0f) << 2] : '=';
        result[output] = '=';
    }
    return result;
}

inline std::string encode(std::string const& data) {
    return encode(data.data(), data.size());
}

namespace detail {

inline int decode_value(unsigned char byte) {
    if (byte >= 'A' && byte <= 'Z') {
        return byte - 'A';
    }
    if (byte >= 'a' && byte <= 'z') {
        return byte - 'a' + 26;
    }
    if (byte >= '0' && byte <= '9') {
        return byte - '0' + 52;
    }
    if (byte == '+') {
        return 62;
    }
    if (byte == '/') {
        return 63;
    }
    return -1;
}

} // namespace detail

// Decode standard Base64 into a binary string. Padding is required for a
// partial final group. Rejects whitespace, invalid characters/padding, and
// nonzero unused padding bits with std::invalid_argument.
// data may be null only when size is zero.
inline std::string decode(void const* data, std::size_t size) {
    if (data == nullptr && size != 0) {
        throw std::invalid_argument("base64::decode: null data with nonzero size");
    }
    if (size % 4 != 0) {
        throw std::invalid_argument("base64::decode: length must be a multiple of four");
    }
    std::string result;
    if (size == 0) {
        return result;
    }
    auto bytes = static_cast<unsigned char const*>(data);
    // Division precedes multiplication to avoid size_t overflow.
    std::size_t output_size = (size / 4) * 3;
    if (bytes[size - 1] == '=') {
        --output_size;
        if (bytes[size - 2] == '=') {
            --output_size;
        }
    }
    if (output_size > result.max_size()) {
        throw std::length_error("base64::decode: output too large");
    }
    result.resize(output_size);
    std::size_t output = 0;
    for (std::size_t i = 0; i < size; i += 4) {
        const int a = detail::decode_value(bytes[i]);
        const int b = detail::decode_value(bytes[i + 1]);
        const bool pad_third = bytes[i + 2] == '=';
        const bool pad_fourth = bytes[i + 3] == '=';
        const int c = pad_third ? 0 : detail::decode_value(bytes[i + 2]);
        const int d = pad_fourth ? 0 : detail::decode_value(bytes[i + 3]);
        if (a < 0 || b < 0 || c < 0 || d < 0) {
            throw std::invalid_argument("base64::decode: invalid character");
        }
        if ((pad_third && !pad_fourth)
            || ((pad_third || pad_fourth) && i != size - 4)
            || (pad_third && (b & 0x0f) != 0)
            || (pad_fourth && !pad_third && (c & 0x03) != 0)) {
            throw std::invalid_argument("base64::decode: invalid padding");
        }
        result[output++] = static_cast<char>((a << 2) | (b >> 4));
        if (!pad_third) {
            result[output++] = static_cast<char>(((b & 0x0f) << 4) | (c >> 2));
        }
        if (!pad_fourth) {
            result[output++] = static_cast<char>(((c & 0x03) << 6) | d);
        }
    }
    return result;
}

inline std::string decode(std::string const& data) {
    return decode(data.data(), data.size());
}

} // namespace base64
} // namespace websocketpp
