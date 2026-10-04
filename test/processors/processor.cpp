// Copyright (c) 2011, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE processors
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

#include <websocketpp/processors/processor.hpp>
#include <websocketpp/http/request.hpp>

BOOST_AUTO_TEST_CASE( exact_match ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(websocketpp::processor::is_websocket_handshake(r));
}

BOOST_AUTO_TEST_CASE( non_match ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(!websocketpp::processor::is_websocket_handshake(r));
}

BOOST_AUTO_TEST_CASE( ci_exact_match ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: UpGrAde\r\nUpgrade: WebSocket\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(websocketpp::processor::is_websocket_handshake(r));
}

BOOST_AUTO_TEST_CASE( non_exact_match1 ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade,foo\r\nUpgrade: websocket,foo\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(websocketpp::processor::is_websocket_handshake(r));
}

BOOST_AUTO_TEST_CASE( non_exact_match2 ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: keep-alive,Upgrade,foo\r\nUpgrade: foo,websocket,bar\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(websocketpp::processor::is_websocket_handshake(r));
}

BOOST_AUTO_TEST_CASE( version_blank ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\nUpgrade: websocket\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(websocketpp::processor::get_websocket_version(r) == -1);
}

BOOST_AUTO_TEST_CASE( version_7 ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 7\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(websocketpp::processor::get_websocket_version(r) == 7);
}

BOOST_AUTO_TEST_CASE( version_8 ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 8\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(websocketpp::processor::get_websocket_version(r) == 8);
}

BOOST_AUTO_TEST_CASE( version_13 ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(websocketpp::processor::get_websocket_version(r) == 13);
}

BOOST_AUTO_TEST_CASE( version_non_numeric ) {
    websocketpp::http::parser::request r;

    std::string handshake = "GET / HTTP/1.1\r\nHost: www.example.com\r\nUpgrade: websocket\r\nSec-WebSocket-Version: abc\r\n\r\n";

    websocketpp::lib::error_code ec;
    r.consume(handshake.c_str(),handshake.size(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());

    BOOST_CHECK(websocketpp::processor::get_websocket_version(r) == -1);
}