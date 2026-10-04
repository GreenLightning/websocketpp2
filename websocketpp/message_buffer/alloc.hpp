// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/memory.hpp>
#include <websocketpp/frame.hpp>

namespace websocketpp {
namespace message_buffer {
namespace alloc {

/// A connection message manager that allocates a new message for each
/// request.
template <typename message>
class con_msg_manager
  : public lib::enable_shared_from_this<con_msg_manager<message> >
{
public:
    typedef con_msg_manager<message> type;
    typedef lib::shared_ptr<con_msg_manager> ptr;
    typedef lib::weak_ptr<con_msg_manager> weak_ptr;

    typedef typename message::ptr message_ptr;

    /// Get an empty message buffer
    /**
     * @return A shared pointer to an empty new message
     */
    message_ptr get_message() {
        return message_ptr(lib::make_shared<message>(type::shared_from_this()));
    }

    /// Get a message buffer with specified size and opcode
    /**
     * @param op The opcode to use
     * @param size Minimum size in bytes to request for the message payload.
     *
     * @return A shared pointer to a new message with specified size.
     */
    message_ptr get_message(frame::opcode::value op,size_t size) {
        return message_ptr(lib::make_shared<message>(type::shared_from_this(),op,size));
    }

    /// Recycle a message
    /**
     * This method shouldn't be called. If it is, return false to indicate an
     * error. The rest of the method recycle chain should notice this and free
     * the memory.
     *
     * @param msg The message to be recycled.
     *
     * @return true if the message was successfully recycled, false otherwse.
     */
    bool recycle(message *) {
        return false;
    }
};

/// An endpoint message manager that allocates a new manager for each
/// connection.
template <typename con_msg_manager>
class endpoint_msg_manager {
public:
    typedef typename con_msg_manager::ptr con_msg_man_ptr;

    /// Get a pointer to a connection message manager
    /**
     * @return A pointer to the requested connection message manager.
     */
    con_msg_man_ptr get_manager() const {
        return con_msg_man_ptr(lib::make_shared<con_msg_manager>());
    }
};

} // namespace alloc
} // namespace message_buffer
} // namespace websocketpp
