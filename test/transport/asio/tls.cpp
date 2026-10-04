// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE transport_asio_tls
#include <boost/test/unit_test.hpp>

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

BOOST_AUTO_TEST_CASE( translated_ec_tls ) {
    typedef websocketpp::transport::asio::tls_socket::connection socket_type;
    websocketpp::lib::asio::error_code ec(1, websocketpp::lib::asio::error::get_ssl_category());
    BOOST_CHECK_EQUAL(socket_type::translate_ec(ec), ec);
    BOOST_CHECK_EQUAL(socket_type::translate_ec(websocketpp::lib::asio::error_code()),
        websocketpp::lib::error_code());
}

BOOST_AUTO_TEST_CASE( server_connection_cleanup ) {
    websocketpp::server<websocketpp::config::asio_tls> s;
}
