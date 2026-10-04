// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE endpoint
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

BOOST_AUTO_TEST_CASE( construct_server_iostream ) {
    websocketpp::server<websocketpp::config::core> s;
}

BOOST_AUTO_TEST_CASE( construct_server_asio_plain ) {
    websocketpp::server<websocketpp::config::asio> s;
}

BOOST_AUTO_TEST_CASE( initialize_server_asio ) {
    websocketpp::server<websocketpp::config::asio> s;
    s.init_asio();
}

BOOST_AUTO_TEST_CASE( initialize_server_asio_external ) {
    websocketpp::server<websocketpp::config::asio> s;
    boost::asio::io_context ios;
    s.init_asio(&ios);
    BOOST_CHECK_EQUAL(&s.get_io_context(), &ios);
    BOOST_CHECK_EQUAL(&s.get_io_service(), &ios);
}

BOOST_AUTO_TEST_CASE( restart_server_asio ) {
    websocketpp::server<websocketpp::config::asio> s;
    s.init_asio();
    BOOST_CHECK_EQUAL(s.run(), 0);
    BOOST_CHECK(s.stopped());

    int calls = 0;
    s.restart();
    s.set_timer(0, [&calls](websocketpp::lib::error_code const & ec) {
        BOOST_CHECK(!ec);
        ++calls;
    });
    s.run();
    BOOST_CHECK_EQUAL(calls, 1);

    s.reset();
    s.set_timer(0, [&calls](websocketpp::lib::error_code const & ec) {
        BOOST_CHECK(!ec);
        ++calls;
    });
    s.run();
    BOOST_CHECK_EQUAL(calls, 2);
}

BOOST_AUTO_TEST_CASE( perpetual_server_asio ) {
    websocketpp::server<websocketpp::config::asio> s;
    s.init_asio();
    s.start_perpetual();
    BOOST_CHECK_EQUAL(s.poll(), 0);
    BOOST_CHECK(!s.stopped());

    s.stop_perpetual();
    BOOST_CHECK_EQUAL(s.run(), 0);
    BOOST_CHECK(s.stopped());
}

BOOST_AUTO_TEST_CASE( listen_invalid_host_service ) {
    websocketpp::server<websocketpp::config::asio> s;
    s.init_asio();
    websocketpp::lib::error_code ec;
    s.listen("127.0.0.1", "websocketpp-invalid-service", ec);
    BOOST_CHECK(ec == websocketpp::transport::asio::error::invalid_host_service);
    BOOST_CHECK(!s.is_listening());
}

#ifdef _WEBSOCKETPP_MOVE_SEMANTICS_
BOOST_AUTO_TEST_CASE( move_construct_server_core ) {
    websocketpp::server<websocketpp::config::core> s1;
    
    websocketpp::server<websocketpp::config::core> s2(std::move(s1));
}

#endif // _WEBSOCKETPP_MOVE_SEMANTICS_

struct endpoint_extension {
    endpoint_extension() : extension_value(5) {}

    int extension_method() {
        return extension_value;
    }

    bool is_server() const {
        return false;
    }

    int extension_value;
};

struct stub_config : public websocketpp::config::core {
    typedef core::concurrency_type concurrency_type;

    typedef core::request_type request_type;
    typedef core::response_type response_type;

    typedef core::message_type message_type;
    typedef core::con_msg_manager_type con_msg_manager_type;
    typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;

    typedef core::alog_type alog_type;
    typedef core::elog_type elog_type;

    typedef core::rng_type rng_type;

    typedef core::transport_type transport_type;

    typedef endpoint_extension endpoint_base;
};

BOOST_AUTO_TEST_CASE( endpoint_extensions ) {
    websocketpp::server<stub_config> s;

    BOOST_CHECK_EQUAL( s.extension_value, 5 );
    BOOST_CHECK_EQUAL( s.extension_method(), 5 );

    BOOST_CHECK( s.is_server() );
}

BOOST_AUTO_TEST_CASE( listen_after_listen_failure ) {
    using websocketpp::transport::asio::error::make_error_code;
    using websocketpp::transport::asio::error::pass_through;

    websocketpp::server<websocketpp::config::asio> server1;
    websocketpp::server<websocketpp::config::asio> server2;

    websocketpp::lib::error_code ec;

    server1.init_asio();
    server2.init_asio();

    boost::asio::ip::tcp::endpoint ep1(boost::asio::ip::make_address("127.0.0.1"), 12345);
    boost::asio::ip::tcp::endpoint ep2(boost::asio::ip::make_address("127.0.0.1"), 23456);

    server1.listen(ep1, ec);
    BOOST_CHECK(!ec);

    // This should return some sort of problem. Usually either "pass through" or
    // a more specific address in use error. It is hard to capture the full range
    // of 'correctly wrong' values.
    server2.listen(ep1, ec);
    BOOST_REQUIRE(ec);

    server2.listen(ep2, ec);
    BOOST_CHECK(!ec);
}
