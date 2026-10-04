// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_CONNECTION_HDL_HPP
#define WEBSOCKETPP_COMMON_CONNECTION_HDL_HPP

#include <websocketpp/common/memory.hpp>

namespace websocketpp {

/// A handle to uniquely identify a connection.
/**
 * This type uniquely identifies a connection. It is implemented as a weak
 * pointer to the connection in question. This provides uniqueness across
 * multiple endpoints and ensures that IDs never conflict or run out.
 *
 * It is safe to make copies of this handle, store those copies in containers,
 * and use them from other threads.
 *
 * This handle can be upgraded to a full shared_ptr using
 * `endpoint::get_con_from_hdl()` from within a handler fired by the connection
 * that owns the handler.
 */
typedef lib::weak_ptr<void> connection_hdl;

} // namespace websocketpp

#endif // WEBSOCKETPP_COMMON_CONNECTION_HDL_HPP
