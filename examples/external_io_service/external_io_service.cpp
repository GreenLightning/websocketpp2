// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#include "tcp_echo_server.hpp"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::lib::error_code;

typedef websocketpp::server<websocketpp::config::asio> ws_echo_server;

// Define a callback to handle incoming messages
void on_message(ws_echo_server* s, websocketpp::connection_hdl hdl, ws_echo_server::message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    // check for a special command to instruct the server to stop listening so
    // it can be cleanly exited.
    if (msg->get_payload() == "stop-listening") {
        s->stop_listening();
        return;
    }

    try {
        s->send(hdl, msg->get_payload(), msg->get_opcode());
    } catch (websocketpp::exception const & e) {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
}

// Define a callback to handle failures accepting connections
void on_end_accept(error_code lib_ec, error_code trans_ec) {
    std::cout << "Accept loop ended "
                << lib_ec.message() << "/" << trans_ec.message() << std::endl;
}

int main() {
    asio::io_context service;

    // Add a TCP echo server on port 9003
    tcp_echo_server custom_http_server(service, 9003);

    // Add a WebSocket echo server on port 9002
    ws_echo_server ws_server;
    ws_server.set_access_channels(websocketpp::log::alevel::all);
    ws_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

    // The only difference in this code between an internal and external
    // io_context is the different constructor to init_asio
    ws_server.init_asio(&service);

    // Register our message handler
    ws_server.set_message_handler(bind(&on_message,&ws_server,::_1,::_2));
    ws_server.listen(9002);
    ws_server.start_accept(&on_end_accept);

    // TODO: add a timer?

    // Start the Asio io_context run loop for all
    service.run();
}