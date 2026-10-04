// Copyright (c) 2026, Green Lightning
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE base64
#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>

#include <websocketpp/base64/base64.hpp>

namespace {

std::string bytes(std::initializer_list<unsigned> input) {
    std::string result;
    for (auto byte : input) {
        result += static_cast<char>(byte);
    }
    return result;
}

} // namespace

BOOST_AUTO_TEST_SUITE ( base64 )

BOOST_AUTO_TEST_CASE( encode_vectors_and_binary ) {
    const char* plain[] = {"", "f", "fo", "foo", "foob", "fooba", "foobar"};
    const char* encoded[] = {"", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==",
                            "Zm9vYmE=", "Zm9vYmFy"};
    for (std::size_t i = 0; i < 7; ++i) {
        const std::string input = plain[i];
        BOOST_REQUIRE_MESSAGE(websocketpp::base64::encode(input) == encoded[i], "Base64 string vector");
        BOOST_REQUIRE_MESSAGE(websocketpp::base64::encode(input.data(), input.size()) == encoded[i],
              "Base64 pointer vector");
        const std::string encoded_input = encoded[i];
        BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode(encoded_input) == input, "Base64 decode string vector");
        BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode(encoded_input.data(), encoded_input.size()) == input,
              "Base64 decode pointer vector");
    }
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::encode(nullptr, 0).empty(), "Base64 null empty");
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::encode(bytes({0, 0, 0})) == "AAAA", "Base64 embedded NULs");
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::encode(bytes({0xff, 0xff, 0xff})) == "////", "Base64 high bytes");
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::encode(bytes({0xfb, 0xef, 0xbe})) == "++++", "Base64 plus alphabet");
    std::string all_bytes;
    for (unsigned i = 0; i < 256; ++i) {
        all_bytes += static_cast<char>(i);
    }
    const std::string binary_encoded =
          "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4v"
          "MDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5f"
          "YGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6P"
          "kJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/"
          "wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v"
          "8PHy8/T19vf4+fr7/P3+/w==";
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::encode(all_bytes) == binary_encoded, "Base64 all byte values");
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode(binary_encoded) == all_bytes, "Base64 decode all byte values");
    BOOST_REQUIRE_THROW(websocketpp::base64::encode(nullptr, 1), std::invalid_argument);
    // Overflow must be detected before reading data or allocating output.
    const char byte = 0;
    BOOST_REQUIRE_THROW(
        websocketpp::base64::encode(&byte, std::numeric_limits<std::size_t>::max()),
        std::length_error);
}

BOOST_AUTO_TEST_CASE( decode_validation_and_round_trips ) {
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode(nullptr, 0).empty(), "Base64 decode null empty");
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode("AAAA") == bytes({0, 0, 0}), "Base64 decode embedded NULs");
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode("////") == bytes({0xff, 0xff, 0xff}), "Base64 decode high bytes");
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode("++++") == bytes({0xfb, 0xef, 0xbe}), "Base64 decode plus alphabet");
    // The pointer overload must respect size without requiring NUL termination.
    const char bounded[] = {'?', 'Z', 'g', '=', '=', '?'};
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode(bounded + 1, 4) == "f", "Base64 decode bounded input");
    BOOST_REQUIRE_THROW(websocketpp::base64::decode(nullptr, 1), std::invalid_argument);

    const char* malformed[] = {
        "A", "AA", "AAA", "AAAAA", "Zg", "Zg=", "Zm8", "Zg===", "Zg======",
        "=AAA", "A=AA", "AA=A", "====", "A===", "=A==", "AA==AAAA",
        "AAA=AAAA", "Zg==Zg==", "AAAA====", "AB==", "AP==", "AAB=", "AAD=",
        "Zm9=", "Zh==", "AA A", "AAA\n", "AAA\r", "AAA\t", "AAAA\n",
        "AAA-", "AAA_", "AAA!"
    };
    for (auto input : malformed) {
        BOOST_REQUIRE_THROW(websocketpp::base64::decode(std::string(input)), std::invalid_argument);
    }

    // Reject every non-alphabet byte (including embedded NUL and high bytes)
    // in every position of a quartet; padding is covered separately above.
    const std::string alphabet =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (unsigned byte = 0; byte < 256; ++byte) {
        const char character = static_cast<char>(byte);
        if (character == '=' || alphabet.find(character) != std::string::npos) {
            continue;
        }
        for (std::size_t position = 0; position < 4; ++position) {
            std::string invalid = "AAAA";
            invalid[position] = character;
            BOOST_REQUIRE_THROW(websocketpp::base64::decode(invalid), std::invalid_argument);
        }
    }
    // Reject every possible nonzero value in the unused padding bits.
    for (std::size_t value = 1; value < 16; ++value) {
        std::string invalid = "AA==";
        invalid[1] = alphabet[value];
        BOOST_REQUIRE_THROW(websocketpp::base64::decode(invalid), std::invalid_argument);
    }
    for (std::size_t value = 1; value < 4; ++value) {
        std::string invalid = "AAA=";
        invalid[2] = alphabet[value];
        BOOST_REQUIRE_THROW(websocketpp::base64::decode(invalid), std::invalid_argument);
    }

    // Exhaust all one-byte and two-byte binary round trips.
    for (unsigned first = 0; first < 256; ++first) {
        const std::string single = bytes({first});
        BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode(websocketpp::base64::encode(single)) == single, "Base64 one-byte round trip");
        for (unsigned second = 0; second < 256; ++second) {
            const std::string pair = bytes({first, second});
            BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode(websocketpp::base64::encode(pair)) == pair, "Base64 two-byte round trip");
        }
    }

    // Deterministic binary data, exercising all final-group sizes and long input.
    std::string input;
    std::uint32_t random_state = 1;
    for (std::size_t size = 0; size <= 1024; ++size) {
        BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode(websocketpp::base64::encode(input)) == input, "Base64 varied-length round trip");
        random_state = random_state * 1664525u + 1013904223u;
        input += static_cast<char>(random_state >> 24);
    }
    input.resize(100000, '\xff');
    BOOST_REQUIRE_MESSAGE(websocketpp::base64::decode(websocketpp::base64::encode(input)) == input, "Base64 large binary round trip");
}

BOOST_AUTO_TEST_SUITE_END()
