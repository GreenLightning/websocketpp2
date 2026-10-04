// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE transport_iostream_endpoint
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <string>

#include <websocketpp/concurrency/none.hpp>
#include <websocketpp/logger/stub.hpp>
#include <websocketpp/transport/iostream/endpoint.hpp>

namespace {

namespace lib = websocketpp::lib;
namespace error = websocketpp::transport::iostream::error;

struct config {
    typedef websocketpp::concurrency::none concurrency_type;
    typedef websocketpp::log::stub alog_type;
    typedef websocketpp::log::stub elog_type;
};

struct test_connection : websocketpp::transport::iostream::connection<config> {
    typedef websocketpp::transport::iostream::connection<config> base;
    typedef lib::shared_ptr<test_connection> ptr;

    test_connection() : base(true, lib::make_shared<config::alog_type>(),
        lib::make_shared<config::elog_type>()) {}

    using base::set_handle;

    lib::error_code write(std::string const & payload) {
        lib::error_code result;
        int callbacks = 0;
        base::async_write(payload.data(), payload.size(),
            [&](lib::error_code const & ec) { result = ec; ++callbacks; });
        BOOST_REQUIRE_EQUAL(callbacks, 1);
        return result;
    }

    lib::error_code shutdown() {
        lib::error_code result;
        int callbacks = 0;
        base::async_shutdown(
            [&](lib::error_code const & ec) { result = ec; ++callbacks; });
        BOOST_REQUIRE_EQUAL(callbacks, 1);
        return result;
    }
};

struct test_endpoint : websocketpp::transport::iostream::endpoint<config> {
    typedef websocketpp::transport::iostream::endpoint<config> base;

    test_endpoint() {
        base::init_logging(lib::make_shared<config::alog_type>(),
            lib::make_shared<config::elog_type>());
    }

    using base::async_connect;

    test_connection::ptr make_connection() {
        test_connection::ptr con = lib::make_shared<test_connection>();
        con->set_handle(con);
        BOOST_REQUIRE(!base::init(con));
        return con;
    }
};

} // namespace

BOOST_AUTO_TEST_CASE( default_connection_has_no_output_or_shutdown_handler ) {
    test_endpoint endpoint;
    test_connection::ptr con = endpoint.make_connection();
    BOOST_CHECK_EQUAL(con->write("hello"), error::make_error_code(error::output_stream_required));
    BOOST_CHECK(!con->shutdown());
}

BOOST_AUTO_TEST_CASE( output_stream_changes_apply_to_future_connections ) {
    test_endpoint endpoint;
    std::stringstream first_output;
    std::stringstream second_output;
    endpoint.register_ostream(&first_output);
    test_connection::ptr first = endpoint.make_connection();
    endpoint.register_ostream(&second_output);
    test_connection::ptr second = endpoint.make_connection();
    endpoint.register_ostream(nullptr);
    test_connection::ptr third = endpoint.make_connection();

    std::string const payload("a\0b", 3);
    BOOST_CHECK(!first->write(payload));
    BOOST_CHECK(!second->write("second"));
    BOOST_CHECK_EQUAL(third->write("third"), error::make_error_code(error::output_stream_required));
    BOOST_CHECK_EQUAL(first_output.str(), payload);
    BOOST_CHECK_EQUAL(second_output.str(), "second");
}

BOOST_AUTO_TEST_CASE( handlers_are_copied_to_future_connections ) {
    test_endpoint endpoint;
    test_connection::ptr before = endpoint.make_connection();
    std::string output;
    websocketpp::connection_hdl write_handle;
    websocketpp::connection_hdl shutdown_handle;
    int writes = 0;
    int shutdowns = 0;
    lib::error_code const expected = error::make_error_code(error::bad_stream);

    endpoint.set_write_handler([&](websocketpp::connection_hdl hdl,
        char const * data, size_t size) {
        ++writes;
        write_handle = hdl;
        output.append(data, size);
        return expected;
    });
    endpoint.set_shutdown_handler([&](websocketpp::connection_hdl hdl) {
        ++shutdowns;
        shutdown_handle = hdl;
        return expected;
    });
    test_connection::ptr configured = endpoint.make_connection();
    endpoint.set_write_handler(nullptr);
    endpoint.set_shutdown_handler(nullptr);
    test_connection::ptr after = endpoint.make_connection();

    BOOST_CHECK_EQUAL(configured->write(std::string("a\0b", 3)), expected);
    BOOST_CHECK_EQUAL(configured->shutdown(), expected);
    BOOST_CHECK_EQUAL(output, std::string("a\0b", 3));
    BOOST_CHECK(write_handle.lock() == configured);
    BOOST_CHECK(shutdown_handle.lock() == configured);

    for (test_connection::ptr con : {before, after}) {
        BOOST_CHECK_EQUAL(con->write("unused"), error::make_error_code(error::output_stream_required));
        BOOST_CHECK(!con->shutdown());
    }
    BOOST_CHECK_EQUAL(writes, 1);
    BOOST_CHECK_EQUAL(shutdowns, 1);
}

BOOST_AUTO_TEST_CASE( endpoint_and_connection_security_flags_are_independent ) {
    test_endpoint endpoint;
    BOOST_CHECK(!endpoint.is_secure());
    endpoint.set_secure(true);
    BOOST_CHECK(endpoint.is_secure());
    test_connection::ptr con = endpoint.make_connection();
    BOOST_CHECK(!con->is_secure());
    con->set_secure(true);
    endpoint.set_secure(false);
    BOOST_CHECK(!endpoint.is_secure());
    BOOST_CHECK(con->is_secure());
    BOOST_CHECK(!endpoint.make_connection()->is_secure());
}

BOOST_AUTO_TEST_CASE( async_connect_reports_success_once ) {
    test_endpoint endpoint;
    test_connection::ptr con = endpoint.make_connection();
    websocketpp::uri_ptr uri = lib::make_shared<websocketpp::uri>("ws://localhost/");
    int callbacks = 0;
    endpoint.async_connect(con, uri, [&](lib::error_code const & ec) {
        ++callbacks;
        BOOST_CHECK(!ec);
    });
    BOOST_CHECK_EQUAL(callbacks, 1);
}
