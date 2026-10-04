// Copyright (c) 2011, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE hybi_util
#include <boost/test/unit_test.hpp>

#include <iostream>

#include "../../src/processors/hybi_util.hpp"
#include "../../src/network_utilities.hpp"

BOOST_AUTO_TEST_CASE( circshift_0 ) {
    if (sizeof(size_t) == 8) {
        size_t test = 0x0123456789abcdef;

        test = websocketpp::processor::hybi_util::circshift_prepared_key(test,0);

        BOOST_CHECK( test == 0x0123456789abcdef);
    } else {
        size_t test = 0x01234567;

        test = websocketpp::processor::hybi_util::circshift_prepared_key(test,0);

        BOOST_CHECK( test == 0x01234567);
    }
}

BOOST_AUTO_TEST_CASE( circshift_1 ) {
    if (sizeof(size_t) == 8) {
        size_t test = 0x0123456789abcdef;

        test = websocketpp::processor::hybi_util::circshift_prepared_key(test,1);

        BOOST_CHECK( test == 0xef0123456789abcd);
    } else {
        size_t test = 0x01234567;

        test = websocketpp::processor::hybi_util::circshift_prepared_key(test,1);

        BOOST_CHECK( test == 0x67012345);
    }
}

BOOST_AUTO_TEST_CASE( circshift_2 ) {
    if (sizeof(size_t) == 8) {
        size_t test = 0x0123456789abcdef;

        test = websocketpp::processor::hybi_util::circshift_prepared_key(test,2);

        BOOST_CHECK( test == 0xcdef0123456789ab);
    } else {
        size_t test = 0x01234567;

        test = websocketpp::processor::hybi_util::circshift_prepared_key(test,2);

        BOOST_CHECK( test == 0x45670123);
    }
}

BOOST_AUTO_TEST_CASE( circshift_3 ) {
    if (sizeof(size_t) == 8) {
        size_t test = 0x0123456789abcdef;

        test = websocketpp::processor::hybi_util::circshift_prepared_key(test,3);

        BOOST_CHECK( test == 0xabcdef0123456789);
    } else {
        size_t test = 0x01234567;

        test = websocketpp::processor::hybi_util::circshift_prepared_key(test,3);

        BOOST_CHECK( test == 0x23456701);
    }
}
