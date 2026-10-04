// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE transport_asio_security
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <websocketpp/common/type_traits.hpp>

#include <websocketpp/transport/asio/security/none.hpp>

static_assert(websocketpp::lib::is_same<websocketpp::lib::error_code,
    websocketpp::lib::asio::error_code>::value,
    "WebSocket++ errors must match the selected Asio backend");

BOOST_AUTO_TEST_CASE( translated_ec_none ) {
    typedef websocketpp::transport::asio::basic_socket::connection socket_type;
    websocketpp::lib::asio::error_code ec = websocketpp::lib::asio::error::connection_reset;
    BOOST_CHECK_EQUAL(socket_type::translate_ec(ec), ec);
    BOOST_CHECK_EQUAL(socket_type::translate_ec(websocketpp::lib::asio::error_code()),
        websocketpp::lib::error_code());
}
