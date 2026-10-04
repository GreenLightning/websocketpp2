// Copyright (c) 2011, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE sha1
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

#include <websocketpp/sha1/sha1.hpp>
#include <websocketpp/utilities.hpp>

BOOST_AUTO_TEST_SUITE ( sha1 )

BOOST_AUTO_TEST_CASE( sha1_test_a ) {
    unsigned char hash[20];
    unsigned char reference[20] = {0xa9, 0x99, 0x3e, 0x36, 0x47,
                                   0x06, 0x81, 0x6a, 0xba, 0x3e,
                                   0x25, 0x71, 0x78, 0x50, 0xc2,
                                   0x6c, 0x9c, 0xd0, 0xd8, 0x9d};

    websocketpp::sha1::calc("abc",3,hash);

    BOOST_CHECK_EQUAL_COLLECTIONS(hash, hash+20, reference, reference+20);
}

BOOST_AUTO_TEST_CASE( sha1_test_b ) {
    unsigned char hash[20];
    unsigned char reference[20] = {0x84, 0x98, 0x3e, 0x44, 0x1c,
                                   0x3b, 0xd2, 0x6e, 0xba, 0xae,
                                   0x4a, 0xa1, 0xf9, 0x51, 0x29,
                                   0xe5, 0xe5, 0x46, 0x70, 0xf1};

    websocketpp::sha1::calc(
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",56,hash);

    BOOST_CHECK_EQUAL_COLLECTIONS(hash, hash+20, reference, reference+20);
}

BOOST_AUTO_TEST_CASE( sha1_test_c ) {
    std::string input;
    unsigned char hash[20];
    unsigned char reference[20] = {0x34, 0xaa, 0x97, 0x3c, 0xd4,
                                   0xc4, 0xda, 0xa4, 0xf6, 0x1e,
                                   0xeb, 0x2b, 0xdb, 0xad, 0x27,
                                   0x31, 0x65, 0x34, 0x01, 0x6f};

    for (int i = 0; i < 1000000; i++) {
        input += 'a';
    }

    websocketpp::sha1::calc(input.c_str(),input.size(),hash);

    BOOST_CHECK_EQUAL_COLLECTIONS(hash, hash+20, reference, reference+20);
}

BOOST_AUTO_TEST_SUITE_END()
