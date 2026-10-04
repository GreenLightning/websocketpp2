// Copyright (c) 2026, Green Lightning
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.
#pragma once

#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <websocketpp/common/system_error.hpp>
#include <websocketpp/logger/levels.hpp>

namespace test_support {

// A shared flag keeps a canceled timer's handler valid during later loop runs.
class deadline {
public:
    explicit deadline(boost::asio::io_context & io, std::chrono::seconds timeout = std::chrono::seconds(5))
        : expired_(std::make_shared<bool>(false))
        , timer_(io, timeout)
    {
        std::shared_ptr<bool> expired = expired_;
        timer_.async_wait([expired, &io](boost::system::error_code const & ec) {
            if (!ec) {
                *expired = true;
                io.stop();
            }
        });
    }

    void cancel() { timer_.cancel(); }
    bool expired() const { return *expired_; }

private:
    std::shared_ptr<bool> expired_;
    boost::asio::steady_timer timer_;
};

template <typename Endpoint>
void silence(Endpoint & endpoint) {
    endpoint.clear_access_channels(websocketpp::log::alevel::all);
    endpoint.clear_error_channels(websocketpp::log::elevel::all);
}

template <typename Endpoint>
boost::asio::ip::tcp::endpoint local_endpoint(Endpoint & endpoint) {
    boost::system::error_code ec;
    boost::asio::ip::tcp::endpoint result = endpoint.get_local_endpoint(ec);
    if (ec) throw boost::system::system_error(ec);
    return result;
}

// Accept only the peer under test, without creating another pending connection
// whose cancellation would also invoke the endpoint's fail handler.
template <typename Endpoint>
void accept_one(Endpoint & endpoint) {
    websocketpp::lib::error_code ec;
    typename Endpoint::connection_ptr con = endpoint.get_connection(ec);
    if (ec) throw std::runtime_error(ec.message());
    endpoint.async_accept(con, [&endpoint, con](websocketpp::lib::error_code ec) {
        if (ec) throw std::runtime_error(ec.message());
        endpoint.stop_listening();
        con->start();
    }, ec);
    if (ec) throw std::runtime_error(ec.message());
}

} // namespace test_support
