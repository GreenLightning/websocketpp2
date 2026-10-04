// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE transport_iostream_base
#include <boost/test/unit_test.hpp>

#include <websocketpp/transport/iostream/base.hpp>

BOOST_AUTO_TEST_CASE( iostream_errors_preserve_category_and_value ) {
    namespace error = websocketpp::transport::iostream::error;
    const error::value values[] = {error::general, error::invalid_num_bytes,
        error::double_read, error::output_stream_required, error::bad_stream};
    for (error::value value : values) {
        websocketpp::lib::error_code ec = value;
        BOOST_CHECK(ec);
        BOOST_CHECK_EQUAL(ec, error::make_error_code(value));
        BOOST_CHECK_EQUAL(ec.value(), static_cast<int>(value));
        BOOST_CHECK(&ec.category() == &error::get_category());
        BOOST_CHECK_EQUAL(ec.category().name(), "websocketpp.transport.iostream");
    }

    websocketpp::lib::error_code iostream_ec = error::general;
    websocketpp::lib::error_code transport_ec = websocketpp::transport::error::general;
    BOOST_REQUIRE_EQUAL(iostream_ec.value(), transport_ec.value());
    BOOST_CHECK(iostream_ec != transport_ec);
}

BOOST_AUTO_TEST_CASE( iostream_errors_have_diagnostic_messages ) {
    namespace error = websocketpp::transport::iostream::error;
    const error::value values[] = {error::general, error::invalid_num_bytes,
        error::double_read, error::output_stream_required, error::bad_stream};
    const char * messages[] = {
        "Generic iostream transport policy error",
        "async_read_at_least call requested more bytes than buffer can store",
        "Async read already in progress",
        "An output stream to be set before async_write can be used",
        "A stream operation returned ios::bad"
    };
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        BOOST_CHECK_EQUAL(error::make_error_code(values[i]).message(), messages[i]);
    }
    BOOST_CHECK_EQUAL(error::get_category().message(999), "Unknown");
}
