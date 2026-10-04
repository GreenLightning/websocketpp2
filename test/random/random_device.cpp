// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE random_device
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <websocketpp/common/stdint.hpp>
#include <websocketpp/random/random_device.hpp>
#include <websocketpp/concurrency/none.hpp>

BOOST_AUTO_TEST_CASE( compiles ) {
    websocketpp::random::random_device::int_generator<int32_t,websocketpp::concurrency::none> rng;

    bool e = false;

    try {
        int32_t foo = rng();
        std::cout << foo << std::endl;
    } catch (...) {
        e = true;
    }

    BOOST_CHECK( e == false );
}
