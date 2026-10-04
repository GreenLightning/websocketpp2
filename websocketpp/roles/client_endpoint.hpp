// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/endpoint.hpp>
#include <websocketpp/uri.hpp>

#include <websocketpp/logger/levels.hpp>

#include <websocketpp/common/system_error.hpp>

#include <string>

namespace websocketpp {

/// Client endpoint role based on the given config
/**
 *
 */
template <typename config>
class client : public endpoint<connection<config>,config> {
public:
    /// Type of this endpoint
    typedef client<config> type;

    /// Type of the endpoint concurrency component
    typedef typename config::concurrency_type concurrency_type;
    /// Type of the endpoint transport component
    typedef typename config::transport_type transport_type;

    /// Type of the connections this server will create
    typedef connection<config> connection_type;
    /// Type of a shared pointer to the connections this server will create
    typedef typename connection_type::ptr connection_ptr;

    /// Type of the connection transport component
    typedef typename transport_type::transport_con_type transport_con_type;
    /// Type of a shared pointer to the connection transport component
    typedef typename transport_con_type::ptr transport_con_ptr;

    /// Type of the endpoint component of this server
    typedef endpoint<connection_type,config> endpoint_type;

    friend class connection<config>;

    explicit client() : endpoint_type(false)
    {
        endpoint_type::m_alog->write(log::alevel::devel, "client constructor");
    }

    /// Get a new connection
    /**
     * Creates and returns a pointer to a new connection to the given URI
     * suitable for passing to connect(connection_ptr). This method allows
     * applying connection specific settings before performing the opening
     * handshake.
     *
     * @param [in] location URI to open the connection to as a uri_ptr
     * @param [out] ec An status code indicating failure reasons, if any
     *
     * @return A connection_ptr to the new connection
     */
    connection_ptr get_connection(uri_ptr location, lib::error_code & ec) {
        if (location->get_secure() && !transport_type::is_secure()) {
            ec = error::make_error_code(error::endpoint_not_secure);
            return connection_ptr();
        }

        connection_ptr con = endpoint_type::create_connection(ec);

        if (!con) {
            // if the transport doesn't have a more specific error, set
            // a generic one.
            if (!ec) {
                ec = error::make_error_code(error::con_creation_failed);
            }
            return con;
        }

        con->set_uri(location);

        ec = lib::error_code();
        return con;
    }

    /// Get a new connection (string version)
    /**
     * Creates and returns a pointer to a new connection to the given URI
     * suitable for passing to connect(connection_ptr). This overload allows
     * default construction of the uri_ptr from a standard string.
     *
     * @param [in] u URI to open the connection to as a string
     * @param [out] ec An status code indicating failure reasons, if any
     *
     * @return A connection_ptr to the new connection
     */
    connection_ptr get_connection(std::string const & u, lib::error_code & ec) {
        uri_ptr location = lib::make_shared<uri>(u);

        if (!location->get_valid()) {
            ec = error::make_error_code(error::invalid_uri);
            return connection_ptr();
        }

        return get_connection(location, ec);
    }

    /// Begin the connection process for the given connection
    /**
     * Initiates the opening connection handshake for connection con. Exact
     * behavior depends on the underlying transport policy.
     *
     * @param con The connection to connect
     *
     * @return The pointer to the connection originally passed in.
     */
    connection_ptr connect(connection_ptr con) {
        // Ask transport to perform a connection
        transport_type::async_connect(
            lib::static_pointer_cast<transport_con_type>(con),
            con->get_uri(),
            lib::bind(
                &type::handle_connect,
                this,
                con,
                lib::placeholders::_1
            )
        );

        return con;
    }
private:
    // handle_connect
    void handle_connect(connection_ptr con, lib::error_code const & ec) {
        if (ec) {
            con->terminate(ec);

            endpoint_type::m_elog->write(log::elevel::rerror,
                    "handle_connect error: "+ec.message());
        } else {
            endpoint_type::m_alog->write(log::alevel::connect,
                "Successful connection");

            con->start();
        }
    }
};

} // namespace websocketpp
