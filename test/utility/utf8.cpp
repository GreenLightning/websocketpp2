// Copyright (c) 2026, Green Lightning
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE utf8
#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <string>

#include <websocketpp/utf8/validator.hpp>

namespace {

std::string bytes(std::initializer_list<unsigned> input) {
    std::string result;
    for (auto byte : input) {
        result += static_cast<char>(byte);
    }
    return result;
}

std::string encode_code_point(std::uint32_t code_point) {
    if (code_point < 0x80) {
        return bytes({code_point});
    }
    if (code_point < 0x800) {
        return bytes({0xc0u | (code_point >> 6), 0x80u | (code_point & 0x3f)});
    }
    if (code_point < 0x10000) {
        return bytes({0xe0u | (code_point >> 12), 0x80u | ((code_point >> 6) & 0x3f),
                      0x80u | (code_point & 0x3f)});
    }
    return bytes({0xf0u | (code_point >> 18), 0x80u | ((code_point >> 12) & 0x3f),
                  0x80u | ((code_point >> 6) & 0x3f), 0x80u | (code_point & 0x3f)});
}

} // namespace

BOOST_AUTO_TEST_SUITE ( utf8 )

BOOST_AUTO_TEST_CASE( validate_and_stream ) {
    websocketpp::utf8::validator state;
    BOOST_REQUIRE_MESSAGE(state.complete(), "UTF-8 initially complete");
    BOOST_REQUIRE_MESSAGE(state.consume(nullptr, 0) && state.complete(), "UTF-8 empty chunk");
    BOOST_REQUIRE_MESSAGE(websocketpp::utf8::validate(""), "UTF-8 empty string");

    // Every Unicode scalar, every truncation, and byte-at-a-time streaming.
    for (std::uint32_t cp = 0; cp <= 0x10ffff; ++cp) {
        const std::string input = encode_code_point(cp);
        if (cp >= 0xd800 && cp <= 0xdfff) {
            BOOST_REQUIRE_MESSAGE(!websocketpp::utf8::validate(input), "UTF-8 surrogate accepted");
            continue;
        }
        BOOST_REQUIRE_MESSAGE(websocketpp::utf8::validate(input), "UTF-8 scalar rejected");
        state.reset();
        for (std::size_t i = 0; i < input.size(); ++i) {
            BOOST_REQUIRE_MESSAGE(state.consume(input.data() + i, 1), "UTF-8 valid chunk rejected");
            BOOST_REQUIRE_MESSAGE(state.complete() == (i + 1 == input.size()), "UTF-8 completion boundary");
            BOOST_REQUIRE_MESSAGE(state.consume(nullptr, 0), "UTF-8 empty chunk in sequence");
            if (i + 1 < input.size()) {
                BOOST_REQUIRE_MESSAGE(!websocketpp::utf8::validate(input.substr(0, i + 1)), "UTF-8 truncation accepted");
            }
        }
    }

    const std::string valid = std::string("ASCII\0", 6)
        + bytes({0xc2, 0x80, 0xdf, 0xbf, 0xe0, 0xa0, 0x80, 0xed, 0x9f, 0xbf,
                 0xee, 0x80, 0x80, 0xef, 0xbb, 0xbf, 0xef, 0xbf, 0xbf,
                 0xf0, 0x90, 0x80, 0x80, 0xf4, 0x8f, 0xbf, 0xbf});
    for (std::size_t split = 0; split <= valid.size(); ++split) {
        state.reset();
        BOOST_REQUIRE_MESSAGE(state.consume(valid.data(), split), "UTF-8 valid prefix");
        BOOST_REQUIRE_MESSAGE(state.consume(valid.data() + split, valid.size() - split) && state.complete(),
              "UTF-8 valid split stream");
    }

    const std::string invalid[] = {
        bytes({0x80}), bytes({0xbf}), bytes({0xc0, 0x80}), bytes({0xc1, 0xbf}),
        bytes({0xc2, 0x7f}), bytes({0xc2, 0xc0}), bytes({0xc2, 0xff}),
        bytes({0xe0, 0x9f, 0xbf}), bytes({0xed, 0xa0, 0x80}),
        bytes({0xed, 0xbf, 0xbf}), bytes({0xe1, 0x80, 0x7f}),
        bytes({0xf0, 0x8f, 0xbf, 0xbf}), bytes({0xf4, 0x90, 0x80, 0x80}),
        bytes({0xf1, 0x80, 0x80, 0xc2}), bytes({0xf5, 0x80, 0x80, 0x80}),
        bytes({0xf8, 0x88, 0x80, 0x80, 0x80}), bytes({0xfc}), bytes({0xfe}),
        bytes({0xff}), bytes({0xe2, 0x82, 0xac, 0x80})
    };
    for (auto const& input : invalid) {
        BOOST_REQUIRE_MESSAGE(!websocketpp::utf8::validate(input), "UTF-8 invalid sequence accepted");
        for (std::size_t split = 0; split <= input.size(); ++split) {
            state.reset();
            const bool first = state.consume(input.data(), split);
            const bool second = state.consume(input.data() + split, input.size() - split);
            BOOST_REQUIRE_MESSAGE(!(first && second) && !state.complete(), "UTF-8 invalid split stream");
            BOOST_REQUIRE_MESSAGE(!state.consume("ok", 2), "UTF-8 invalid state not sticky");
            BOOST_REQUIRE_MESSAGE(!state.consume(nullptr, 0), "UTF-8 empty chunk cleared error");
            state.reset();
            BOOST_REQUIRE_MESSAGE(state.complete() && state.consume("ok", 2) && state.complete(),
                  "UTF-8 reset after error");
        }
    }

    // Exhaust every possible one-byte and two-byte input.
    for (unsigned first = 0; first < 256; ++first) {
        BOOST_REQUIRE_MESSAGE(websocketpp::utf8::validate(bytes({first})) == (first < 128), "UTF-8 one-byte input");
        for (unsigned second = 0; second < 256; ++second) {
            const bool expected = (first < 128 && second < 128)
                || (first >= 0xc2 && first <= 0xdf && second >= 0x80 && second <= 0xbf);
            BOOST_REQUIRE_MESSAGE(websocketpp::utf8::validate(bytes({first, second})) == expected, "UTF-8 two-byte input");
        }
    }
    state.reset();
    BOOST_REQUIRE_MESSAGE(state.consume("\xf0", 1) && !state.complete(), "UTF-8 partial sequence");
    state.reset();
    BOOST_REQUIRE_MESSAGE(state.complete() && state.consume("a", 1) && state.complete(),
          "UTF-8 reset partial sequence");
    BOOST_REQUIRE_THROW(state.consume(nullptr, 1), std::invalid_argument);
    BOOST_REQUIRE_MESSAGE(state.complete(), "UTF-8 null argument changed state");
}

BOOST_AUTO_TEST_SUITE_END()
