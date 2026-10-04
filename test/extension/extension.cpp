// Copyright (c) 2011, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE extension
#include <boost/test/unit_test.hpp>

#include <websocketpp/error.hpp>
#include <websocketpp/extensions/extension.hpp>

BOOST_AUTO_TEST_CASE( extension_errors_preserve_category_and_value ) {
    namespace error = websocketpp::extensions::error;
    const error::value values[] = {error::general, error::disabled};
    for (error::value value : values) {
        // Implicit enum conversion must use the extension category too.
        websocketpp::lib::error_code ec = value;
        BOOST_CHECK(ec);
        BOOST_CHECK_EQUAL(ec, error::make_error_code(value));
        BOOST_CHECK_EQUAL(ec.value(), static_cast<int>(value));
        BOOST_CHECK(&ec.category() == &error::get_category());
        BOOST_CHECK_EQUAL(ec.category().name(), "websocketpp.extension");
    }

    websocketpp::lib::error_code extension_ec = error::general;
    websocketpp::lib::error_code library_ec = websocketpp::error::general;
    BOOST_REQUIRE_EQUAL(extension_ec.value(), library_ec.value());
    BOOST_CHECK(extension_ec != library_ec);
}

BOOST_AUTO_TEST_CASE( extension_errors_have_diagnostic_messages ) {
    namespace error = websocketpp::extensions::error;
    BOOST_CHECK_EQUAL(error::make_error_code(error::general).message(),
        "Generic extension error");
    BOOST_CHECK_EQUAL(error::make_error_code(error::disabled).message(),
        "Use of methods from disabled extension");
    BOOST_CHECK(error::get_category().message(999).find("Unknown") != std::string::npos);
}
