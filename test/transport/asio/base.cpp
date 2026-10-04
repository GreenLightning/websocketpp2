// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE transport_asio_base
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <websocketpp/transport/asio/base.hpp>

BOOST_AUTO_TEST_CASE( blank_error ) {
    websocketpp::lib::error_code ec;

    BOOST_CHECK( !ec );
}

BOOST_AUTO_TEST_CASE( asio_error ) {
    using websocketpp::transport::asio::error::make_error_code;
    using websocketpp::transport::asio::error::general;

    websocketpp::lib::error_code ec = make_error_code(general);

    BOOST_CHECK( ec == general );
    BOOST_CHECK( ec.value() == 1 );
}
