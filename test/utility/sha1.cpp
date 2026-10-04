// Copyright (c) 2011, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE sha1
#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>

#include <websocketpp/sha1/sha1.hpp>

namespace {

std::string hex(websocketpp::sha1::digest const& digest) {
    static char const digits[] = "0123456789abcdef";
    std::string result;
    for (auto byte : digest) {
        result += digits[byte >> 4];
        result += digits[byte & 15];
    }
    return result;
}

} // namespace

BOOST_AUTO_TEST_SUITE ( sha1 )

BOOST_AUTO_TEST_CASE( sha1_test_a ) {
    unsigned char reference[20] = {0xa9, 0x99, 0x3e, 0x36, 0x47,
                                   0x06, 0x81, 0x6a, 0xba, 0x3e,
                                   0x25, 0x71, 0x78, 0x50, 0xc2,
                                   0x6c, 0x9c, 0xd0, 0xd8, 0x9d};

    websocketpp::sha1::digest const hash = websocketpp::sha1::calc("abc", 3);

    BOOST_CHECK_EQUAL_COLLECTIONS(hash.begin(), hash.end(), reference, reference+20);
}

BOOST_AUTO_TEST_CASE( sha1_test_b ) {
    unsigned char reference[20] = {0x84, 0x98, 0x3e, 0x44, 0x1c,
                                   0x3b, 0xd2, 0x6e, 0xba, 0xae,
                                   0x4a, 0xa1, 0xf9, 0x51, 0x29,
                                   0xe5, 0xe5, 0x46, 0x70, 0xf1};

    websocketpp::sha1::digest const hash = websocketpp::sha1::calc(
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56);

    BOOST_CHECK_EQUAL_COLLECTIONS(hash.begin(), hash.end(), reference, reference+20);
}

BOOST_AUTO_TEST_CASE( sha1_test_c ) {
    std::string input;
    unsigned char reference[20] = {0x34, 0xaa, 0x97, 0x3c, 0xd4,
                                   0xc4, 0xda, 0xa4, 0xf6, 0x1e,
                                   0xeb, 0x2b, 0xdb, 0xad, 0x27,
                                   0x31, 0x65, 0x34, 0x01, 0x6f};

    for (int i = 0; i < 1000000; i++) {
        input += 'a';
    }

    websocketpp::sha1::digest const hash = websocketpp::sha1::calc(input.data(), input.size());

    BOOST_CHECK_EQUAL_COLLECTIONS(hash.begin(), hash.end(), reference, reference+20);
}

BOOST_AUTO_TEST_CASE( calc_empty_binary_and_padding ) {
    BOOST_REQUIRE_MESSAGE(hex(websocketpp::sha1::calc(nullptr, 0)) ==
          "da39a3ee5e6b4b0d3255bfef95601890afd80709", "SHA-1 empty");

    // Binary fixtures from Python hashlib, covering padding/block boundaries.
    struct fixture { std::size_t size; char const* expected; };
    const fixture fixtures[] = {
        {1, "5ba93c9db0cff93f52b521d7420e43f6eda2784f"},
        {55, "8ae2d46729cfe68ff927af5eec9c7d1b66d65ac2"},
        {56, "636e2ec698dac903498e648bd2f3af641d3c88cb"},
        {57, "7cb1330f35244b57437539253304ea78a6b7c443"},
        {63, "6d942da0c4392b123528f2905c713a3ce28364bd"},
        {64, "c6138d514ffa2135bfce0ed0b8fac65669917ec7"},
        {65, "69bd728ad6e13cd76ff19751fde427b00e395746"},
        {119, "41c89d06001bab4ab78736b44efe7ce18ce6ae08"},
        {120, "d3dbd653bd8597b7475321b60a36891278e6a04a"},
        {127, "89d7312a903f65cd2b3e34a975e55dbea9033353"},
        {128, "e6434bc401f98603d7eda504790c98c67385d535"},
        {129, "3352e41cc30b40ae80108970492b21014049e625"},
        {1024, "5b00669c480d5cffbdfa8bdba99561160f2d1b77"}
    };
    for (auto const& fixture : fixtures) {
        std::string input;
        for (std::size_t i = 0; i < fixture.size; ++i) {
            input += static_cast<char>(i % 256);
        }
        // Deliberately pass an unaligned input address.
        const std::string unaligned = "x" + input;
        BOOST_REQUIRE_MESSAGE(hex(websocketpp::sha1::calc(unaligned.data() + 1, input.size())) == fixture.expected,
              "SHA-1 binary/padding fixture");
    }
    BOOST_REQUIRE_THROW(websocketpp::sha1::calc(nullptr, 1), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()
