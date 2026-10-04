// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/asio.hpp>
#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/functional.hpp>
#include <websocketpp/common/system_error.hpp>
#include <websocketpp/common/platforms.hpp>
#include <websocketpp/common/connection_hdl.hpp>

#include <string>

// Interface that sockets/security policies must implement

/*
 * Endpoint Interface
 *
 * bool is_secure() const;
 * @return Whether or not the endpoint creates secure connections
 *
 * lib::error_code init(socket_con_ptr scon);
 * Called by the transport after a new connection is created to initialize
 * the socket component of the connection.
 * @param scon Pointer to the socket component of the connection
 * @return Error code (empty on success)
 */


// Connection
// TODO
// set_hostname(std::string hostname)
// pre_init(init_handler);
// post_init(init_handler);

namespace websocketpp {
namespace transport {
namespace asio {
namespace socket {

typedef lib::function<void(lib::asio::error_code const &)> shutdown_handler;

/**
 * The transport::asio::socket::* classes are a set of security/socket related
 * policies and support code for the ASIO transport types.
 */

/// Errors related to asio transport sockets
namespace error {
    enum value {
        /// Catch-all error for security policy errors that don't fit in other
        /// categories
        security = 1,

        /// Catch-all error for socket component errors that don't fit in other
        /// categories
        socket,

        /// A function was called in a state that it was illegal to do so.
        invalid_state,

        /// The application was prompted to provide a TLS context and it was
        /// empty or otherwise invalid
        invalid_tls_context,

        /// TLS Handshake Timeout
        tls_handshake_timeout,

        /// pass_through from underlying library
        pass_through,

        /// Required tls_init handler not present
        missing_tls_init_handler,

        /// TLS Handshake Failed
        tls_handshake_failed,
        
        /// Failed to set TLS SNI hostname
        tls_failed_sni_hostname
    };
} // namespace error

/// Error category related to asio transport socket policies
class socket_category : public lib::error_category {
public:
    char const * name() const noexcept {
        return "websocketpp.transport.asio.socket";
    }

    std::string message(int value) const {
        switch(value) {
            case error::security:
                return "Security policy error";
            case error::socket:
                return "Socket component error";
            case error::invalid_state:
                return "Invalid state";
            case error::invalid_tls_context:
                return "Invalid or empty TLS context supplied";
            case error::tls_handshake_timeout:
                return "TLS handshake timed out";
            case error::pass_through:
                return "Pass through from socket policy";
            case error::missing_tls_init_handler:
                return "Required tls_init handler not present.";
            case error::tls_handshake_failed:
                return "TLS handshake failed";
            case error::tls_failed_sni_hostname:
                return "Failed to set TLS SNI hostname";
            default:
                return "Unknown";
        }
    }
};

inline lib::error_category const & get_socket_category() {
    static socket_category instance;
    return instance;
}

inline lib::error_code make_error_code(error::value e) {
    return lib::error_code(static_cast<int>(e), get_socket_category());
}

/// Type of asio transport socket policy initialization handlers
typedef lib::function<void(const lib::error_code&)> init_handler;

} // namespace socket
} // namespace asio
} // namespace transport
} // namespace websocketpp
