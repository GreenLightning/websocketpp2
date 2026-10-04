// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#include "connection_tu2.hpp"

void echo_func(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    s->send(hdl, msg->get_payload(), msg->get_opcode());
}

std::string run_server_test(std::string input, bool log) {
    server test_server;
    return run_server_test(test_server,input,log);
}

std::string run_server_test(server & s, std::string input, bool log) {
    server::connection_ptr con;
    std::stringstream output;

    if (log) {
        s.set_access_channels(websocketpp::log::alevel::all);
        s.set_error_channels(websocketpp::log::elevel::all);
    } else {
        s.clear_access_channels(websocketpp::log::alevel::all);
        s.clear_error_channels(websocketpp::log::elevel::all);
    }

    s.register_ostream(&output);

    con = s.get_connection();
    con->start();

    std::stringstream channel;

    channel << input;
    channel >> *con;

    return output.str();
}
