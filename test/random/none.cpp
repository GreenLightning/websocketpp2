// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE random_none
#include <boost/test/unit_test.hpp>

#include <websocketpp/common/stdint.hpp>
#include <websocketpp/random/none.hpp>

BOOST_AUTO_TEST_CASE( does_it_compile ) {
    websocketpp::random::none::int_generator<int32_t> rng;

    int32_t foo = rng();

    BOOST_CHECK( foo == 0 );
}
