// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <algorithm>
#include <string>

namespace websocketpp {
namespace utility {

inline std::string to_hex(std::string const & input) {
    return to_hex(input.c_str(), input.size());
}

inline std::string to_hex(uint8_t const * input, size_t length) {
    std::string output;
    output.reserve(length * 3);
    char const * hex = "0123456789ABCDEF";

    for (size_t i = 0; i < length; i++) {
        output += hex[(input[i] & 0xF0) >> 4];
        output += hex[input[i] & 0x0F];
        output += ' ';
    }

    return output;
}

inline std::string to_hex(const char* input,size_t length) {
    return to_hex(reinterpret_cast<const uint8_t*>(input),length);
}

inline std::string string_replace_all(std::string subject, std::string const &
    search, std::string const & replace)
{
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

} // namespace utility
} // namespace websocketpp
