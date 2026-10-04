// Copyright (c) 2026, Green Lightning
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE transport_asio_proxy
#include <boost/test/unit_test.hpp>

#include "test_support.hpp"
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/http/request.hpp>
#include <array>
#include <functional>

namespace {

typedef websocketpp::lib::asio::ip::tcp tcp;
typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::server<websocketpp::config::asio> server;

// A single-use HTTP proxy. Successful CONNECT requests forward actual bytes
// between the client and an independent WebSocket server.
class proxy {
public:
    enum behavior { tunnel, reject, stall };

    proxy(websocketpp::lib::asio::io_context & io, behavior mode, tcp::endpoint destination)
        : acceptor_(io, tcp::endpoint(websocketpp::lib::asio::ip::address_v4::loopback(), 0))
        , peer_(io), upstream_(io), mode_(mode), destination_(destination)
    {}

    std::string uri() const {
        return "http://127.0.0.1:" + std::to_string(acceptor_.local_endpoint().port());
    }

    void start() {
        acceptor_.async_accept(peer_, [this](websocketpp::lib::error_code ec) {
            BOOST_REQUIRE(!ec);
            acceptor_.close();
            websocketpp::lib::asio::async_read_until(peer_, request_buffer_, "\r\n\r\n",
                [this](websocketpp::lib::error_code ec, size_t size) {
                    BOOST_REQUIRE(!ec);
                    std::string raw(size, '\0');
                    std::istream stream(&request_buffer_);
                    stream.read(&raw[0], size);
                    websocketpp::http::parser::request request;
                    websocketpp::lib::error_code parse_ec;
                    request.consume(raw.data(), raw.size(), parse_ec);
                    BOOST_REQUIRE(!parse_ec);
                    BOOST_REQUIRE(request.ready());
                    ++requests;
                    BOOST_CHECK_EQUAL(request.get_method(), "CONNECT");
                    BOOST_CHECK_EQUAL(request.get_uri(), "127.0.0.1:"
                        + std::to_string(destination_.port()));
                    BOOST_CHECK_EQUAL(request.get_header("Proxy-Authorization"),
                        "Basic dXNlcjpwYXNz");
                    unexpected_bytes += request_buffer_.size();
                    if (mode_ == tunnel) {
                        upstream_.async_connect(destination_, [this](websocketpp::lib::error_code ec) {
                            BOOST_REQUIRE(!ec);
                            respond("HTTP/1.1 200 Connection established\r\n\r\n");
                        });
                    } else if (mode_ == reject) {
                        respond("HTTP/1.1 407 Proxy Authentication Required\r\nContent-Length: 0\r\n\r\n");
                    } else {
                        await_disconnect();
                    }
                });
        });
    }

    int requests = 0;
    size_t unexpected_bytes = 0;
    bool disconnected = false;
    std::function<void()> on_disconnect;

private:
    void respond(std::string response) {
        response_ = response;
        websocketpp::lib::asio::async_write(peer_, websocketpp::lib::asio::buffer(response_),
            [this](websocketpp::lib::error_code ec, size_t) {
                BOOST_REQUIRE(!ec);
                if (mode_ == tunnel) {
                    forward(peer_, upstream_, client_buffer_);
                    forward(upstream_, peer_, server_buffer_);
                } else {
                    await_disconnect();
                }
            });
    }

    void await_disconnect() {
        peer_.async_read_some(websocketpp::lib::asio::buffer(client_buffer_),
            [this](websocketpp::lib::error_code ec, size_t size) {
                unexpected_bytes += size;
                BOOST_CHECK(ec == websocketpp::lib::asio::error::eof
                    || ec == websocketpp::lib::asio::error::connection_reset);
                finish();
            });
    }

    void forward(tcp::socket & source, tcp::socket & sink, std::array<char, 4096> & buffer) {
        source.async_read_some(websocketpp::lib::asio::buffer(buffer),
            [this, &source, &sink, &buffer](websocketpp::lib::error_code ec, size_t size) {
                if (ec) { finish(); return; }
                websocketpp::lib::asio::async_write(sink, websocketpp::lib::asio::buffer(buffer.data(), size),
                    [this, &source, &sink, &buffer](websocketpp::lib::error_code ec, size_t) {
                        if (ec) { finish(); return; }
                        forward(source, sink, buffer);
                    });
            });
    }

    void finish() {
        if (disconnected) return;
        disconnected = true;
        websocketpp::lib::error_code ignored;
        peer_.close(ignored);
        upstream_.close(ignored);
        if (on_disconnect) on_disconnect();
    }

    tcp::acceptor acceptor_;
    tcp::socket peer_, upstream_;
    behavior mode_;
    tcp::endpoint destination_;
    websocketpp::lib::asio::streambuf request_buffer_;
    std::string response_;
    std::array<char, 4096> client_buffer_, server_buffer_;
};

client::connection_ptr proxied_connection(client & endpoint, proxy & intermediary,
    unsigned short destination_port)
{
    websocketpp::lib::error_code ec;
    client::connection_ptr con = endpoint.get_connection("ws://127.0.0.1:"
        + std::to_string(destination_port) + "/echo", ec);
    BOOST_REQUIRE(!ec);
    BOOST_REQUIRE(con);
    con->set_proxy(intermediary.uri(), ec);
    BOOST_REQUIRE(!ec);
    con->set_proxy_basic_auth("user", "pass", ec);
    BOOST_REQUIRE(!ec);
    return con;
}

void check_proxy_failure(proxy::behavior behavior, websocketpp::lib::error_code expected) {
    websocketpp::lib::asio::io_context io;
    test_support::deadline deadline(io);
    client endpoint;
    test_support::silence(endpoint);
    endpoint.init_asio(&io);
    proxy intermediary(io, behavior, tcp::endpoint(websocketpp::lib::asio::ip::address_v4::loopback(), 1234));
    int failures = 0, opens = 0, messages = 0, closes = 0;
    auto finish = [&] { if (failures && intermediary.disconnected) deadline.cancel(); };
    intermediary.on_disconnect = finish;
    endpoint.set_fail_handler([&](websocketpp::connection_hdl) { ++failures; finish(); });
    endpoint.set_open_handler([&](websocketpp::connection_hdl) { ++opens; });
    endpoint.set_close_handler([&](websocketpp::connection_hdl) { ++closes; });
    endpoint.set_message_handler([&](websocketpp::connection_hdl, client::message_ptr) { ++messages; });
    client::connection_ptr con = proxied_connection(endpoint, intermediary, 1234);
    websocketpp::lib::error_code ec;
    con->set_proxy_timeout(100, ec);
    BOOST_REQUIRE(!ec);
    intermediary.start();
    endpoint.connect(con);
    io.run();
    BOOST_CHECK(!deadline.expired());
    BOOST_CHECK_EQUAL(intermediary.requests, 1);
    BOOST_CHECK(intermediary.disconnected);
    BOOST_CHECK_EQUAL(intermediary.unexpected_bytes, 0u);
    BOOST_CHECK_EQUAL(failures, 1);
    BOOST_CHECK_EQUAL(opens, 0);
    BOOST_CHECK_EQUAL(messages, 0);
    BOOST_CHECK_EQUAL(closes, 0);
    BOOST_CHECK_EQUAL(con->get_ec(), expected);
    BOOST_CHECK(con->get_state() == websocketpp::session::state::closed);
}

} // namespace

BOOST_AUTO_TEST_CASE( authenticated_connect_proxy_echo ) {
    websocketpp::lib::asio::io_context io;
    test_support::deadline deadline(io);
    server upstream;
    client endpoint;
    test_support::silence(upstream);
    test_support::silence(endpoint);
    upstream.init_asio(&io);
    endpoint.init_asio(&io);
    upstream.listen(tcp::endpoint(websocketpp::lib::asio::ip::address_v4::loopback(), 0));
    proxy intermediary(io, proxy::tunnel, test_support::local_endpoint(upstream));
    int opens = 0, closes = 0, failures = 0, server_messages = 0, client_messages = 0;
    std::string const payload("proxy\0echo", 10);
    auto finish = [&] { if (closes == 2 && intermediary.disconnected) deadline.cancel(); };
    intermediary.on_disconnect = finish;
    upstream.set_open_handler([&](websocketpp::connection_hdl) { ++opens; });
    endpoint.set_open_handler([&](websocketpp::connection_hdl hdl) {
        ++opens;
        endpoint.send(hdl, payload, websocketpp::frame::opcode::BINARY);
    });
    upstream.set_message_handler([&](websocketpp::connection_hdl hdl, server::message_ptr msg) {
        ++server_messages;
        BOOST_CHECK_EQUAL(msg->get_payload(), payload);
        BOOST_CHECK_EQUAL(msg->get_opcode(), websocketpp::frame::opcode::BINARY);
        upstream.send(hdl, msg);
    });
    endpoint.set_message_handler([&](websocketpp::connection_hdl hdl, client::message_ptr msg) {
        ++client_messages;
        BOOST_CHECK_EQUAL(msg->get_payload(), payload);
        BOOST_CHECK_EQUAL(msg->get_opcode(), websocketpp::frame::opcode::BINARY);
        endpoint.close(hdl, websocketpp::close::status::normal, "done");
    });
    auto closed = [&](websocketpp::connection_hdl) { ++closes; finish(); };
    upstream.set_close_handler(closed);
    endpoint.set_close_handler(closed);
    auto failed = [&](websocketpp::connection_hdl) { ++failures; io.stop(); };
    upstream.set_fail_handler(failed);
    endpoint.set_fail_handler(failed);
    client::connection_ptr con = proxied_connection(endpoint, intermediary, test_support::local_endpoint(upstream).port());
    test_support::accept_one(upstream);
    intermediary.start();
    endpoint.connect(con);
    io.run();
    BOOST_CHECK(!deadline.expired());
    BOOST_CHECK_EQUAL(intermediary.requests, 1);
    BOOST_CHECK_EQUAL(intermediary.unexpected_bytes, 0u);
    BOOST_CHECK(intermediary.disconnected);
    BOOST_CHECK_EQUAL(opens, 2);
    BOOST_CHECK_EQUAL(closes, 2);
    BOOST_CHECK_EQUAL(failures, 0);
    BOOST_CHECK_EQUAL(server_messages, 1);
    BOOST_CHECK_EQUAL(client_messages, 1);
    BOOST_CHECK(!con->get_ec());
    BOOST_CHECK_EQUAL(con->get_remote_close_code(), websocketpp::close::status::normal);
}

BOOST_AUTO_TEST_CASE( proxy_rejects_connect_with_407 ) {
    check_proxy_failure(proxy::reject, websocketpp::transport::asio::error::make_error_code(
        websocketpp::transport::asio::error::proxy_failed));
}

BOOST_AUTO_TEST_CASE( stalled_connect_proxy_times_out ) {
    check_proxy_failure(proxy::stall, websocketpp::transport::error::make_error_code(
        websocketpp::transport::error::timeout));
}
