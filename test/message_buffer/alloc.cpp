// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE message_buffer_alloc
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

#include <websocketpp/message_buffer/alloc.hpp>

template <template <class> class con_msg_manager>
struct stub {
    typedef websocketpp::lib::shared_ptr<stub> ptr;

    typedef con_msg_manager<stub> con_msg_man_type;
    typedef typename con_msg_man_type::ptr con_msg_man_ptr;
    typedef typename con_msg_man_type::weak_ptr con_msg_man_weak_ptr;

    stub(con_msg_man_ptr manager, websocketpp::frame::opcode::value op, size_t size = 128)
      : m_opcode(op)
      , m_manager(manager)
      , m_size(size) {}

    bool recycle() {
        con_msg_man_ptr shared = m_manager.lock();

        if (shared) {
            return shared->recycle(this);
        } else {
            return false;
        }
    }

    websocketpp::frame::opcode::value   m_opcode;
    con_msg_man_weak_ptr                m_manager;
    size_t                              m_size;
};

BOOST_AUTO_TEST_CASE( basic_get_message ) {
    typedef stub<websocketpp::message_buffer::alloc::con_msg_manager>
        message_type;
    typedef websocketpp::message_buffer::alloc::con_msg_manager<message_type>
        con_msg_man_type;

    con_msg_man_type::ptr manager(new con_msg_man_type());
    message_type::ptr msg = manager->get_message(websocketpp::frame::opcode::TEXT,512);

    BOOST_CHECK(msg);
    BOOST_CHECK(msg->m_opcode == websocketpp::frame::opcode::TEXT);
    BOOST_CHECK(msg->m_manager.lock() == manager);
    BOOST_CHECK(msg->m_size == 512);
}

BOOST_AUTO_TEST_CASE( basic_get_manager ) {
    typedef stub<websocketpp::message_buffer::alloc::con_msg_manager>
        message_type;
    typedef websocketpp::message_buffer::alloc::con_msg_manager<message_type>
        con_msg_man_type;
    typedef websocketpp::message_buffer::alloc::endpoint_msg_manager
        <con_msg_man_type> endpoint_manager_type;

    endpoint_manager_type em;
    con_msg_man_type::ptr manager = em.get_manager();
    message_type::ptr msg = manager->get_message(websocketpp::frame::opcode::TEXT,512);

    BOOST_CHECK(msg);
    BOOST_CHECK(msg->m_opcode == websocketpp::frame::opcode::TEXT);
    BOOST_CHECK(msg->m_manager.lock() == manager);
    BOOST_CHECK(msg->m_size == 512);
}

