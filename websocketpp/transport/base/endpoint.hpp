// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_TRANSPORT_BASE_HPP
#define WEBSOCKETPP_TRANSPORT_BASE_HPP

#include <websocketpp/common/functional.hpp>
#include <websocketpp/common/system_error.hpp>

namespace websocketpp {
/// Transport policies provide network connectivity and timers
/**
 * ### Endpoint Interface
 *
 * Transport endpoint components needs to provide:
 *
 * **init**\n
 * `lib::error_code init(transport_con_ptr tcon)`\n
 * init is called by an endpoint once for each newly created connection.
 * It's purpose is to give the transport policy the chance to perform any
 * transport specific initialization that couldn't be done via the default
 * constructor.
 *
 * **is_secure**\n
 * `bool is_secure() const`\n
 * Test whether the transport component of this endpoint is capable of secure
 * connections.
 *
 * **async_connect**\n
 * `void async_connect(transport_con_ptr tcon, uri_ptr location,
 *  connect_handler handler)`\n
 * Initiate a connection to `location` using the given connection `tcon`. `tcon`
 * is a pointer to the transport connection component of the connection. When
 * complete, `handler` should be called with the the connection's
 * `connection_hdl` and any error that occurred.
 * note: optional, client transports only
 *
 * **async_accept**\n
 * todo:
 * note: optional, server transports only
 * 
 * **init_logging**
 * `void init_logging(const lib::shared_ptr<alog_type>& a, const lib::shared_ptr<elog_type>& e)`\n
 * Called once after construction to provide pointers to the endpoint's access
 * and error loggers. These may be stored and used to log messages or ignored.
 * 
 * 
 * **is_listening**
 * `bool is_listening()`
 * Server roles only. Called to determine if the server is listening and that calls to
 * start_accept / async_accept are valid.
 */
namespace transport {

/// The type and signature of the callback passed to the accept method
typedef lib::function<void(lib::error_code const &)> accept_handler;

/// The type and signature of the callback passed to the connect method
typedef lib::function<void(lib::error_code const &)> connect_handler;

} // namespace transport
} // namespace websocketpp

#endif // WEBSOCKETPP_TRANSPORT_BASE_HPP
