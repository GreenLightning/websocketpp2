// Copyright (c) 2026, Green Lightning
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace websocketpp {
namespace utf8 {

inline bool validate(std::string const& text);

class validator {
public:
    validator();

    // Retains state across chunks. False means invalid UTF-8; true may
    // mean incomplete. Invalid input stays invalid until reset().
    // data may be null only when size is zero.
    bool consume(void const* data, std::size_t size);
    bool complete() const;
    void reset();

private:
    std::uint8_t remaining_;
    std::uint8_t next_min_;
    std::uint8_t next_max_;
    bool valid_;
};

inline validator::validator() {
    reset();
}

inline void validator::reset() {
    remaining_ = 0;
    next_min_ = 0x80;
    next_max_ = 0xbf;
    valid_ = true;
}

inline bool validator::complete() const {
    return valid_ && remaining_ == 0;
}

inline bool validator::consume(void const* data, std::size_t size) {
    if (data == nullptr && size != 0) {
        throw std::invalid_argument("utf8::validator::consume: null data with nonzero size");
    }
    if (!valid_) {
        return false;
    }
    auto bytes = static_cast<unsigned char const*>(data);
    for (std::size_t i = 0; i < size; ++i) {
        const unsigned char byte = bytes[i];
        if (remaining_ != 0) {
            if (byte < next_min_ || byte > next_max_) {
                valid_ = false;
                return false;
            }
            --remaining_;
            next_min_ = 0x80;
            next_max_ = 0xbf;
        } else if (byte <= 0x7f) {
            continue;
        } else if (byte >= 0xc2 && byte <= 0xdf) {
            remaining_ = 1;
        } else if (byte >= 0xe0 && byte <= 0xef) {
            remaining_ = 2;
            // Exclude overlong encodings and UTF-16 surrogates.
            next_min_ = byte == 0xe0 ? 0xa0 : 0x80;
            next_max_ = byte == 0xed ? 0x9f : 0xbf;
        } else if (byte >= 0xf0 && byte <= 0xf4) {
            remaining_ = 3;
            // Exclude overlong encodings and values above U+10FFFF.
            next_min_ = byte == 0xf0 ? 0x90 : 0x80;
            next_max_ = byte == 0xf4 ? 0x8f : 0xbf;
        } else {
            valid_ = false;
            return false;
        }
    }
    return true;
}

inline bool validate(std::string const& text) {
    validator state;
    return state.consume(text.data(), text.size()) && state.complete();
}

} // namespace utf8
} // namespace websocketpp
