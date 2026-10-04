// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE transport_asio_base
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <websocketpp/transport/asio/base.hpp>

BOOST_AUTO_TEST_CASE( blank_error ) {
    websocketpp::lib::error_code ec;

    BOOST_CHECK( !ec );
}

BOOST_AUTO_TEST_CASE( asio_error ) {
    using websocketpp::transport::asio::error::make_error_code;
    using websocketpp::transport::asio::error::general;

    websocketpp::lib::error_code ec = make_error_code(general);

    BOOST_CHECK( ec == general );
    BOOST_CHECK( ec.value() == 1 );
}

BOOST_AUTO_TEST_CASE( executor_bound_handler_allocator ) {
    namespace asio = websocketpp::lib::asio;
    websocketpp::transport::asio::handler_allocator allocator;
    void * storage = allocator.allocate(1);
    allocator.deallocate(storage);

    asio::io_context context;
    asio::strand<asio::io_context::executor_type> strand(context.get_executor());
    asio::steady_timer timer(context, std::chrono::milliseconds(0));
    int calls = 0;
    timer.async_wait(asio::bind_executor(strand,
        websocketpp::transport::asio::make_custom_alloc_handler(allocator,
            [&calls](websocketpp::lib::error_code const & ec) {
                BOOST_CHECK(!ec);
                ++calls;
            })));

    // Asio must reserve the handler's storage until the operation completes.
    void * busy = allocator.allocate(1);
    BOOST_CHECK(busy != storage);
    allocator.deallocate(busy);
    context.run();
    BOOST_CHECK_EQUAL(calls, 1);

    void * recycled = allocator.allocate(1);
    BOOST_CHECK(recycled == storage);
    allocator.deallocate(recycled);
}
