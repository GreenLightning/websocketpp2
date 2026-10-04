// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE transport_asio_base
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <websocketpp/common/type_traits.hpp>

#include <websocketpp/transport/asio/security/none.hpp>
#include <websocketpp/transport/asio/security/tls.hpp>

template <typename base>
struct dummy_con : public base {
	websocketpp::lib::error_code test() {
		return this->translate_ec(websocketpp::lib::asio::error_code());
	}
};

BOOST_AUTO_TEST_CASE( translated_ec_none ) {
    dummy_con<websocketpp::transport::asio::basic_socket::connection> tscon;

	// If the current configuration settings result in the library error type and the asio
	// error type being the same, then the code should pass through natively. Otherwise
	// we should get a generic pass through error.
	if(websocketpp::lib::is_same<websocketpp::lib::error_code,websocketpp::lib::asio::error_code>::value) {
    	BOOST_CHECK_EQUAL( tscon.test(), websocketpp::lib::error_code() );
    } else {
    	BOOST_CHECK_EQUAL( tscon.test(), websocketpp::transport::error::make_error_code(websocketpp::transport::error::pass_through) );
    }
}

BOOST_AUTO_TEST_CASE( translated_ec_tls ) {
    dummy_con<websocketpp::transport::asio::tls_socket::connection> tscon;

	// If the current configuration settings result in the library error type and the asio
	// error type being the same, then the code should pass through natively. Otherwise
	// we should get a generic pass through error.
	if(websocketpp::lib::is_same<websocketpp::lib::error_code,websocketpp::lib::asio::error_code>::value) {
    	BOOST_CHECK_EQUAL( tscon.test(), websocketpp::lib::error_code() );
    } else {
    	BOOST_CHECK_EQUAL( tscon.test(), websocketpp::transport::error::make_error_code(websocketpp::transport::error::pass_through) );
    }
}
