// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE close
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

#include <websocketpp/close.hpp>
#include <websocketpp/utilities.hpp>

using namespace websocketpp;

BOOST_AUTO_TEST_CASE( reserved_values ) {
    BOOST_CHECK( !close::status::reserved(999) );
    BOOST_CHECK( close::status::reserved(1004) );
    BOOST_CHECK( close::status::reserved(1016) );
    BOOST_CHECK( close::status::reserved(2999) );
    BOOST_CHECK( !close::status::reserved(1000) );
}

BOOST_AUTO_TEST_CASE( invalid_values ) {
    BOOST_CHECK( close::status::invalid(0) );
    BOOST_CHECK( close::status::invalid(999) );
    BOOST_CHECK( !close::status::invalid(1000) );
    BOOST_CHECK( close::status::invalid(1005) );
    BOOST_CHECK( close::status::invalid(1006) );
    BOOST_CHECK( close::status::invalid(1015) );
    BOOST_CHECK( !close::status::invalid(2999) );
    BOOST_CHECK( !close::status::invalid(3000) );
    BOOST_CHECK( close::status::invalid(5000) );
}

BOOST_AUTO_TEST_CASE( value_extraction ) {
    lib::error_code ec;
    std::string payload = "oo";

    // Value = 1000
    payload[0] = 0x03;
    payload[1] = char(0xe8);
    BOOST_CHECK( close::extract_code(payload,ec) == close::status::normal );
    BOOST_CHECK( !ec );

    // Value = 1004
    payload[0] = 0x03;
    payload[1] = char(0xec);
    BOOST_CHECK( close::extract_code(payload,ec) == 1004 );
    BOOST_CHECK( ec == error::reserved_close_code );

    // Value = 1005
    payload[0] = 0x03;
    payload[1] = char(0xed);
    BOOST_CHECK( close::extract_code(payload,ec) == close::status::no_status );
    BOOST_CHECK( ec == error::invalid_close_code );

    // Value = 3000
    payload[0] = 0x0b;
    payload[1] = char(0xb8);
    BOOST_CHECK( close::extract_code(payload,ec) == 3000 );
    BOOST_CHECK( !ec );
}

BOOST_AUTO_TEST_CASE( extract_empty ) {
    lib::error_code ec;
    std::string payload;

    BOOST_CHECK( close::extract_code(payload,ec) == close::status::no_status );
    BOOST_CHECK( !ec );
}

BOOST_AUTO_TEST_CASE( extract_short ) {
    lib::error_code ec;
    std::string payload = "0";

    BOOST_CHECK( close::extract_code(payload,ec) == close::status::protocol_error );
    BOOST_CHECK( ec == error::bad_close_code );
}

BOOST_AUTO_TEST_CASE( extract_reason ) {
    lib::error_code ec;
    std::string payload = "00Foo";

    BOOST_CHECK( close::extract_reason(payload,ec) == "Foo" );
    BOOST_CHECK( !ec );

    payload.clear();
    BOOST_CHECK( close::extract_reason(payload,ec).empty() );
    BOOST_CHECK( !ec );

    payload = "00";
    BOOST_CHECK( close::extract_reason(payload,ec).empty() );
    BOOST_CHECK( !ec );

    payload = "000";
    payload[2] = char(0xFF);

    close::extract_reason(payload,ec);
    BOOST_CHECK( ec == error::invalid_utf8 );
}
