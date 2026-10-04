// Copyright (c) 2011, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE extension
#include <boost/test/unit_test.hpp>

#include <string>

#include <websocketpp/extensions/extension.hpp>

BOOST_AUTO_TEST_CASE( blank ) {
    BOOST_CHECK( true );
}
