// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE message
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

#include <websocketpp/message_buffer/message.hpp>

template <typename message>
struct stub {
    typedef websocketpp::lib::weak_ptr<stub> weak_ptr;
    typedef websocketpp::lib::shared_ptr<stub> ptr;

    stub() : recycled(false) {}

    bool recycle(message *) {
        this->recycled = true;
        return false;
    }

    bool recycled;
};

BOOST_AUTO_TEST_CASE( basic_size_check ) {
    typedef websocketpp::message_buffer::message<stub> message_type;
    typedef stub<message_type> stub_type;

    stub_type::ptr s(new stub_type());
    message_type::ptr msg(new message_type(s,websocketpp::frame::opcode::TEXT,500));

    BOOST_CHECK(msg->get_payload().capacity() >= 500);
}

BOOST_AUTO_TEST_CASE( recycle ) {
    typedef websocketpp::message_buffer::message<stub> message_type;
    typedef stub<message_type> stub_type;

    stub_type::ptr s(new stub_type());
    message_type::ptr msg(new message_type(s,websocketpp::frame::opcode::TEXT,500));

    BOOST_CHECK(s->recycled == false);
    BOOST_CHECK(msg->recycle() == false);
    BOOST_CHECK(s->recycled == true);
}

