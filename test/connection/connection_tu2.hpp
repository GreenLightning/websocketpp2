// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#include <iostream>
#include <sstream>

// Test Environment:
// server, no TLS, no locks, iostream based transport
#include <websocketpp/config/debug.hpp>
#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>

typedef websocketpp::server<websocketpp::config::core> server;
/// NOTE: the "server" config is being used for the client here because we don't
/// want to pull in the real RNG. A better way to do this might be a custom
/// client config with the RNG explicitly stubbed out.
typedef websocketpp::client<websocketpp::config::core> client;
typedef websocketpp::config::core::message_type::ptr message_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

void echo_func(server* s, websocketpp::connection_hdl hdl, message_ptr msg);
std::string run_server_test(std::string input, bool log = false);
std::string run_server_test(server & s, std::string input, bool log = false);
