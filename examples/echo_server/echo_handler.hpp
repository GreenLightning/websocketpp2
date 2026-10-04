// Copyright (c) 2012, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

class echo_handler : public server::handler {
    void on_message(connection_ptr con, std::string msg) {
        con->write(msg);
    }
};
