// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE error
#include <boost/test/unit_test.hpp>

#include <websocketpp/error.hpp>

BOOST_AUTO_TEST_CASE( constructing_exceptions ) {
    websocketpp::lib::error_code test_ec = websocketpp::error::make_error_code(websocketpp::error::test);
    websocketpp::lib::error_code general_ec = websocketpp::error::make_error_code(websocketpp::error::general);

    websocketpp::exception b("foo");
    websocketpp::exception c("foo",test_ec);
    websocketpp::exception d("");
    websocketpp::exception e("",test_ec);

    BOOST_CHECK_EQUAL(b.what(),"foo");
    BOOST_CHECK_EQUAL(b.code(),general_ec);

    BOOST_CHECK_EQUAL(c.what(),"foo");
    BOOST_CHECK_EQUAL(c.code(),test_ec);

    BOOST_CHECK_EQUAL(d.what(),"Generic error");
    BOOST_CHECK_EQUAL(d.code(),general_ec);

    BOOST_CHECK_EQUAL(e.what(),"Test Error");
    BOOST_CHECK_EQUAL(e.code(),test_ec);
}

