// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE endpoint_tls
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <vector>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

BOOST_AUTO_TEST_CASE( construct_server_asio_tls ) {
    websocketpp::server<websocketpp::config::asio_tls> s;
}

/*
// temporary disable because library doesn't pass
BOOST_AUTO_TEST_CASE( emplace ) {
    std::stringstream out1;
    std::stringstream out2;

    std::vector<websocketpp::server<websocketpp::config::asio_tls>> v;

    v.emplace_back();
    v.emplace_back();

    v[0].get_alog().set_ostream(&out1);
    v[0].get_alog().set_ostream(&out2);

    v[0].get_alog().write(websocketpp::log::alevel::app,"devel0");
    v[1].get_alog().write(websocketpp::log::alevel::app,"devel1");
    BOOST_CHECK( out1.str().size() > 0 );
    BOOST_CHECK( out2.str().size() > 0 );
}*/
