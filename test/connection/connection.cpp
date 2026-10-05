// Copyright (c) 2011, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE connection
#include <boost/test/unit_test.hpp>

#include "connection_tu2.hpp"

// Include special debugging transport
//#include <websocketpp/config/minimal_client.hpp>
#include <websocketpp/transport/debug/endpoint.hpp>

namespace {

// Hold frame writes until the test explicitly completes them. HTTP handshake
// writes still use the ordinary iostream transport.
template <typename Config>
class controlled_write_connection : public websocketpp::transport::iostream::connection<Config> {
public:
    typedef websocketpp::transport::iostream::connection<Config> base;
    typedef websocketpp::lib::shared_ptr<controlled_write_connection> ptr;

    controlled_write_connection(bool is_server,
        websocketpp::lib::shared_ptr<typename Config::alog_type> const & alog,
        websocketpp::lib::shared_ptr<typename Config::elog_type> const & elog)
        : base(is_server, alog, elog) {}

    void complete_write(websocketpp::lib::error_code ec) {
        BOOST_REQUIRE(pending_write);
        websocketpp::transport::write_handler handler = pending_write;
        pending_write = websocketpp::transport::write_handler();
        handler(ec);
    }

    int writes = 0;
    std::string written;

protected:
    using base::async_write;

    void async_write(std::vector<websocketpp::transport::buffer> const & buffers,
        websocketpp::transport::write_handler handler)
    {
        BOOST_REQUIRE(!pending_write);
        ++writes;
        for (auto const & buffer : buffers) written.append(buffer.buf, buffer.len);
        pending_write = handler;
    }

private:
    websocketpp::transport::write_handler pending_write;
};

template <typename Config>
struct controlled_write_endpoint : websocketpp::transport::iostream::endpoint<Config> {
    typedef controlled_write_connection<Config> transport_con_type;
    typedef typename transport_con_type::ptr transport_con_ptr;
};

struct controlled_write_config : websocketpp::config::core {
    typedef controlled_write_endpoint<core::transport_config> transport_type;
};

} // namespace

BOOST_AUTO_TEST_CASE( send_before_open_does_not_write_data ) {
    client endpoint;
    std::stringstream output;
    endpoint.register_ostream(&output);
    websocketpp::lib::error_code ec;
    client::connection_ptr con = endpoint.get_connection("ws://localhost", ec);
    BOOST_REQUIRE(!ec);
    BOOST_REQUIRE(con);
    BOOST_CHECK(con->get_state() == websocketpp::session::state::connecting);
    BOOST_CHECK_EQUAL(con->send(std::string("early"), websocketpp::frame::opcode::BINARY),
        websocketpp::error::make_error_code(websocketpp::error::invalid_state));
    BOOST_CHECK(output.str().empty());

    endpoint.connect(con);
    std::string const handshake = output.str();
    BOOST_REQUIRE(!handshake.empty());
    BOOST_CHECK(con->get_state() == websocketpp::session::state::connecting);
    BOOST_CHECK_EQUAL(con->send(std::string("still early"), websocketpp::frame::opcode::BINARY),
        websocketpp::error::make_error_code(websocketpp::error::invalid_state));
    BOOST_CHECK_EQUAL(output.str(), handshake);
}

BOOST_AUTO_TEST_CASE( write_failure_stops_queued_messages ) {
    typedef websocketpp::server<controlled_write_config> endpoint_type;
    endpoint_type endpoint;
    endpoint.clear_access_channels(websocketpp::log::alevel::all);
    endpoint.clear_error_channels(websocketpp::log::elevel::all);
    std::stringstream output;
    endpoint.register_ostream(&output);
    int opens = 0, closes = 0, failures = 0;
    endpoint.set_open_handler([&](websocketpp::connection_hdl) { ++opens; });
    endpoint.set_close_handler([&](websocketpp::connection_hdl) { ++closes; });
    endpoint.set_fail_handler([&](websocketpp::connection_hdl) { ++failures; });
    websocketpp::lib::error_code ec;
    endpoint_type::connection_ptr con = endpoint.get_connection(ec);
    BOOST_REQUIRE(!ec);
    BOOST_REQUIRE(con);
    con->start();
    std::string const handshake = "GET / HTTP/1.1\r\nHost: localhost\r\n"
        "Connection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    BOOST_REQUIRE_EQUAL(con->read_all(handshake.data(), handshake.size()), handshake.size());
    BOOST_REQUIRE(con->get_state() == websocketpp::session::state::open);
    BOOST_REQUIRE_EQUAL(opens, 1);
    BOOST_REQUIRE(!con->send(std::string("first"), websocketpp::frame::opcode::BINARY));
    BOOST_REQUIRE(!con->send(std::string("second"), websocketpp::frame::opcode::BINARY));
    BOOST_REQUIRE(!con->send(std::string("third"), websocketpp::frame::opcode::BINARY));
    BOOST_CHECK_EQUAL(con->writes, 1);
    BOOST_CHECK_EQUAL(con->written, std::string("\x82\x05" "first", 7));
    BOOST_CHECK_EQUAL(con->get_buffered_amount(), 11u);

    websocketpp::lib::error_code const error = websocketpp::transport::error::make_error_code(
        websocketpp::transport::error::pass_through);
    con->complete_write(error);
    BOOST_CHECK_EQUAL(con->get_ec(), error);
    BOOST_CHECK(con->get_state() == websocketpp::session::state::closed);
    BOOST_CHECK_EQUAL(closes, 1);
    BOOST_CHECK_EQUAL(failures, 0);
    BOOST_CHECK_EQUAL(con->writes, 1);
    BOOST_CHECK_EQUAL(con->written, std::string("\x82\x05" "first", 7));
    BOOST_CHECK_EQUAL(con->send(std::string("after failure"), websocketpp::frame::opcode::BINARY),
        websocketpp::error::make_error_code(websocketpp::error::invalid_state));
    BOOST_CHECK_EQUAL(con->writes, 1);
    BOOST_CHECK_EQUAL(closes, 1);
}

// NOTE: these tests currently test against hardcoded output values. I am not
// sure how problematic this will be. If issues arise like order of headers the
// output should be parsed by http::response and have values checked directly

BOOST_AUTO_TEST_CASE( basic_http_request ) {
    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
    std::string output = "HTTP/1.1 426 Upgrade Required\r\nServer: " +
                         std::string(websocketpp::user_agent)+"\r\n\r\n";

    std::string o2 = run_server_test(input);

    BOOST_CHECK(o2 == output);
}

struct connection_extension {
    connection_extension() : extension_value(5) {}

    int extension_method() {
        return extension_value;
    }

    bool is_server() const {
        return false;
    }

    int extension_value;
};

struct stub_config : public websocketpp::config::core {
    typedef core::concurrency_type concurrency_type;

    typedef core::request_type request_type;
    typedef core::response_type response_type;

    typedef core::message_type message_type;
    typedef core::con_msg_manager_type con_msg_manager_type;
    typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;

    typedef core::alog_type alog_type;
    typedef core::elog_type elog_type;

    typedef core::rng_type rng_type;

    typedef core::transport_type transport_type;

    typedef core::endpoint_base endpoint_base;
    typedef connection_extension connection_base;
};

struct debug_config_client : public websocketpp::config::core {
    typedef debug_config_client type;
    
    typedef core::concurrency_type concurrency_type;

    typedef core::request_type request_type;
    typedef core::response_type response_type;

    typedef core::message_type message_type;
    typedef core::con_msg_manager_type con_msg_manager_type;
    typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;

    typedef core::alog_type alog_type;
    typedef core::elog_type elog_type;

    typedef websocketpp::random::none::int_generator<uint32_t> rng_type;

    struct transport_config {
        typedef type::concurrency_type concurrency_type;
        typedef type::elog_type elog_type;
        typedef type::alog_type alog_type;
        typedef type::request_type request_type;
        typedef type::response_type response_type;

        /// Controls compile time enabling/disabling of thread syncronization
        /// code Disabling can provide a minor performance improvement to single
        /// threaded applications
        static bool const enable_multithreading = true;

        /// Default timer values (in ms)
        static const long timeout_socket_pre_init = 5000;
        static const long timeout_proxy = 5000;
        static const long timeout_socket_post_init = 5000;
        static const long timeout_connect = 5000;
        static const long timeout_socket_shutdown = 5000;
    };

    /// Transport Endpoint Component
    typedef websocketpp::transport::debug::endpoint<transport_config>
        transport_type;

    typedef core::endpoint_base endpoint_base;
    typedef connection_extension connection_base;
    
    static const websocketpp::log::level elog_level = websocketpp::log::elevel::none;
    static const websocketpp::log::level alog_level = websocketpp::log::alevel::none;
};

struct connection_setup {
    connection_setup(bool p_is_server)
            : alog(websocketpp::lib::make_shared<stub_config::alog_type>())
            , elog(websocketpp::lib::make_shared<stub_config::elog_type>())
            , c(p_is_server, "", alog, elog, rng) {}

    websocketpp::lib::error_code ec;
    websocketpp::lib::shared_ptr<stub_config::alog_type> alog;
    websocketpp::lib::shared_ptr<stub_config::elog_type> elog;
    stub_config::rng_type rng;
    websocketpp::connection<stub_config> c;
};

typedef websocketpp::client<debug_config_client> debug_client;
typedef websocketpp::server<debug_config_client> debug_server;

/*void echo_func(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    s->send(hdl, msg->get_payload(), msg->get_opcode());
}*/

void validate_func(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    s->send(hdl, msg->get_payload(), msg->get_opcode());
}

bool validate_set_ua(server* s, websocketpp::connection_hdl hdl) {
    server::connection_ptr con = s->get_con_from_hdl(hdl);
    con->replace_header("Server","foo");
    return true;
}

void http_func(server* s, websocketpp::connection_hdl hdl) {
    using namespace websocketpp::http;

    server::connection_ptr con = s->get_con_from_hdl(hdl);

    std::string res = con->get_resource();

    con->set_body(res);
    con->set_status(status_code::ok);

    BOOST_CHECK_EQUAL(con->get_response_code(), status_code::ok);
    BOOST_CHECK_EQUAL(con->get_response_msg(), status_code::get_string(status_code::ok));
}

void http_func_with_move(server* s, websocketpp::connection_hdl hdl) {
    using namespace websocketpp::http;

    server::connection_ptr con = s->get_con_from_hdl(hdl);

    std::string res = con->get_resource();

    con->set_body(std::move(res));
    con->set_status(status_code::ok);

    BOOST_CHECK_EQUAL(con->get_response_code(), status_code::ok);
    BOOST_CHECK_EQUAL(con->get_response_msg(), status_code::get_string(status_code::ok));
}

void defer_http_func(server* s, bool * deferred, websocketpp::connection_hdl hdl) {
    *deferred = true;
    
    server::connection_ptr con = s->get_con_from_hdl(hdl);
    
    websocketpp::lib::error_code ec = con->defer_http_response();
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());
}

void check_on_fail(server* s, websocketpp::lib::error_code ec, bool & called, 
    websocketpp::connection_hdl hdl)
{
    server::connection_ptr con = s->get_con_from_hdl(hdl);

    BOOST_CHECK_EQUAL(ec, con->get_ec());
    called = true;
}

void on_open_print(server* s, websocketpp::connection_hdl hdl)
{
    server::connection_ptr con = s->get_con_from_hdl(hdl);

    std::cout << con->get_uri() << std::endl;
}

void fail_on_open(websocketpp::connection_hdl) {
    BOOST_CHECK(false);
}
void fail_on_http(websocketpp::connection_hdl) {
    BOOST_CHECK(false);
}

BOOST_AUTO_TEST_CASE( connection_extensions ) {
    connection_setup env(true);

    BOOST_CHECK( env.c.extension_value == 5 );
    BOOST_CHECK( env.c.extension_method() == 5 );

    BOOST_CHECK( env.c.is_server() == true );
}

BOOST_AUTO_TEST_CASE( basic_websocket_request ) {
    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nOrigin: http://www.example.com\r\n\r\n";
    std::string output = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\nServer: ";
    output+=websocketpp::user_agent;
    output+="\r\nUpgrade: websocket\r\n\r\n";

    server s;
    s.set_message_handler(bind(&echo_func,&s,::_1,::_2));

    BOOST_CHECK(run_server_test(s,input) == output);
}

BOOST_AUTO_TEST_CASE( supported_websocket_versions ) {
    connection_setup env(true);
    std::vector<int> const & versions = env.c.get_supported_versions();
    BOOST_REQUIRE_EQUAL(versions.size(), 1);
    BOOST_CHECK_EQUAL(versions[0], 13);
}

BOOST_AUTO_TEST_CASE( draft_websocket_requests_rejected ) {
    int const versions[] = {0, 7, 8};
    for (size_t i = 0; i < sizeof(versions)/sizeof(versions[0]); ++i) {
        std::stringstream input;
        input << "GET / HTTP/1.1\r\nHost: www.example.com\r\n"
              << "Connection: Upgrade\r\nUpgrade: websocket\r\n"
              << "Sec-WebSocket-Version: " << versions[i] << "\r\n"
              << "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";

        server s;
        s.set_user_agent("test");
        s.set_open_handler(&fail_on_open);
        s.set_http_handler(&fail_on_http);
        bool called = false;
        websocketpp::lib::error_code ec = make_error_code(websocketpp::error::unsupported_version);
        s.set_fail_handler(bind(&check_on_fail,&s,ec,websocketpp::lib::ref(called),::_1));

        BOOST_CHECK_EQUAL(run_server_test(s,input.str()),
            "HTTP/1.1 400 Bad Request\r\nSec-WebSocket-Version: 13\r\nServer: test\r\n\r\n");
        BOOST_CHECK(called);
    }
}

BOOST_AUTO_TEST_CASE( versionless_websocket_requests_rejected ) {
    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\n"
        "Connection: Upgrade\r\nUpgrade: websocket\r\n"
        "Sec-WebSocket-Key1: 3e6b263  4 17 80\r\n"
        "Sec-WebSocket-Key2: 17  9 G`ZD9   2 2b 7X 3 /r90\r\n\r\n";

    // Reject as soon as the headers arrive, with or without the legacy challenge.
    for (int i = 0; i < 2; ++i) {
        server s;
        s.set_user_agent("test");
        s.set_open_handler(&fail_on_open);
        s.set_http_handler(&fail_on_http);
        bool called = false;
        websocketpp::lib::error_code ec = make_error_code(websocketpp::error::invalid_version);
        s.set_fail_handler(bind(&check_on_fail,&s,ec,websocketpp::lib::ref(called),::_1));

        BOOST_CHECK_EQUAL(run_server_test(s,input),
            "HTTP/1.1 400 Bad Request\r\nServer: test\r\n\r\n");
        BOOST_CHECK(called);
        input += "WjN}|M(6";
    }
}

template <int version>
struct draft_client_config : public websocketpp::config::core {
    static const int client_version = version;
};

template <int version>
void check_draft_client_rejected() {
    typedef websocketpp::client<draft_client_config<version> > draft_client;
    draft_client c;
    c.clear_access_channels(websocketpp::log::alevel::all);
    c.clear_error_channels(websocketpp::log::elevel::all);
    std::stringstream output;
    c.register_ostream(&output);

    bool failed = false;
    c.set_fail_handler([&failed](websocketpp::connection_hdl) { failed = true; });
    websocketpp::lib::error_code ec;
    typename draft_client::connection_ptr con = c.get_connection("ws://localhost",ec);
    BOOST_REQUIRE(!ec);
    c.connect(con);

    BOOST_CHECK(failed);
    BOOST_CHECK_EQUAL(con->get_ec(), make_error_code(websocketpp::error::unsupported_version));
    BOOST_CHECK_EQUAL(con->get_state(), websocketpp::session::state::closed);
    BOOST_CHECK(output.str().empty());
}

BOOST_AUTO_TEST_CASE( draft_client_versions_rejected ) {
    check_draft_client_rejected<0>();
    check_draft_client_rejected<7>();
    check_draft_client_rejected<8>();
}

BOOST_AUTO_TEST_CASE( http_request ) {
    std::string input = "GET /foo/bar HTTP/1.1\r\nHost: www.example.com\r\nOrigin: http://www.example.com\r\n\r\n";
    std::string output = "HTTP/1.1 200 OK\r\nContent-Length: 8\r\nServer: ";
    output+=websocketpp::user_agent;
    output+="\r\n\r\n/foo/bar";

    server s;
    s.set_http_handler(bind(&http_func,&s,::_1));

    BOOST_CHECK_EQUAL(run_server_test(s,input), output);
}

BOOST_AUTO_TEST_CASE( http_request_with_custom_status ) {
    std::string const input = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
    std::string const message = "Queued for processing";
    server s;
    s.set_user_agent("");
    bool called = false;
    s.set_http_handler([&](websocketpp::connection_hdl hdl) {
        called = true;
        server::connection_ptr con = s.get_con_from_hdl(hdl);
        con->set_status(websocketpp::http::status_code::accepted, message);
        con->set_body("queued");
        BOOST_CHECK_EQUAL(con->get_response_code(), websocketpp::http::status_code::accepted);
        BOOST_CHECK_EQUAL(con->get_response_msg(), message);
    });

    BOOST_CHECK_EQUAL(run_server_test(s, input),
        "HTTP/1.1 202 Queued for processing\r\nContent-Length: 6\r\n\r\nqueued");
    BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE( custom_status_from_invalid_state_throws ) {
    server s;
    websocketpp::lib::error_code creation_ec;
    server::connection_ptr con = s.get_connection(creation_ec);
    BOOST_REQUIRE(!creation_ec);
    BOOST_REQUIRE(con);
    BOOST_CHECK_EXCEPTION(
        con->set_status(websocketpp::http::status_code::accepted, "Queued for processing"),
        websocketpp::exception,
        [](websocketpp::exception const & e) {
            return e.code() == websocketpp::error::make_error_code(websocketpp::error::invalid_state);
        });
}

BOOST_AUTO_TEST_CASE( http_request_with_move ) {
    std::string input = "GET /foo/bar HTTP/1.1\r\nHost: www.example.com\r\nOrigin: http://www.example.com\r\n\r\n";
    std::string output = "HTTP/1.1 200 OK\r\nContent-Length: 8\r\nServer: ";
    output+=websocketpp::user_agent;
    output+="\r\n\r\n/foo/bar";

    server s;
    s.set_http_handler(bind(&http_func_with_move,&s,::_1));

    BOOST_CHECK_EQUAL(run_server_test(s,input), output);
}

BOOST_AUTO_TEST_CASE( deferred_http_request ) {
    std::string input = "GET /foo/bar HTTP/1.1\r\nHost: www.example.com\r\nOrigin: http://www.example.com\r\n\r\n";
    std::string output = "HTTP/1.1 200 OK\r\nContent-Length: 8\r\nServer: ";
    output+=websocketpp::user_agent;
    output+="\r\n\r\n/foo/bar";

    server s;
    server::connection_ptr con;
    bool deferred = false;
    s.set_http_handler(bind(&defer_http_func,&s, &deferred,::_1));

    s.clear_access_channels(websocketpp::log::alevel::all);
    s.clear_error_channels(websocketpp::log::elevel::all);
    
    std::stringstream ostream;
    s.register_ostream(&ostream);

    websocketpp::lib::error_code ec;
    con = s.get_connection(ec);
    BOOST_REQUIRE(!ec);
    BOOST_REQUIRE(con);
    con->start();
    
    BOOST_CHECK(!deferred);
    BOOST_CHECK_EQUAL(ostream.str(), "");
    con->read_some(input.data(),input.size());
    BOOST_CHECK(deferred);
    BOOST_CHECK_EQUAL(ostream.str(), "");

    con->set_body(con->get_resource());
    con->set_status(websocketpp::http::status_code::ok);
    
    s.send_http_response(con->get_handle(),ec);
    BOOST_CHECK_EQUAL(ec, websocketpp::lib::error_code());
    BOOST_CHECK_EQUAL(ostream.str(), output);
    con->send_http_response(ec);
    BOOST_CHECK_EQUAL(ec, make_error_code(websocketpp::error::invalid_state));
    
}

BOOST_AUTO_TEST_CASE( request_no_server_header ) {
    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nOrigin: http://www.example.com\r\n\r\n";
    std::string output = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\nUpgrade: websocket\r\n\r\n";

    server s;
    s.set_user_agent("");
    s.set_message_handler(bind(&echo_func,&s,::_1,::_2));

    BOOST_CHECK_EQUAL(run_server_test(s,input), output);
}

BOOST_AUTO_TEST_CASE( request_no_server_header_override ) {
    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nOrigin: http://www.example.com\r\n\r\n";
    std::string output = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\nServer: foo\r\nUpgrade: websocket\r\n\r\n";

    server s;
    s.set_user_agent("");
    s.set_message_handler(bind(&echo_func,&s,::_1,::_2));
    s.set_validate_handler(bind(&validate_set_ua,&s,::_1));

    BOOST_CHECK_EQUAL(run_server_test(s,input), output);
}

BOOST_AUTO_TEST_CASE( basic_client_websocket ) {
    std::string uri = "ws://localhost";

    //std::string output = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\nServer: foo\r\nUpgrade: websocket\r\n\r\n";

    std::string ref = "GET / HTTP/1.1\r\nConnection: Upgrade\r\nFoo: Bar\r\nHost: localhost\r\nSec-WebSocket-Key: AAAAAAAAAAAAAAAAAAAAAA==\r\nSec-WebSocket-Version: 13\r\nUpgrade: websocket\r\nUser-Agent: foo\r\n\r\n";

    std::stringstream output;

    client e;
    e.set_access_channels(websocketpp::log::alevel::none);
    e.set_error_channels(websocketpp::log::elevel::none);
    e.set_user_agent("foo");
    e.register_ostream(&output);

    client::connection_ptr con;
    websocketpp::lib::error_code ec;
    con = e.get_connection(uri, ec);
    con->append_header("Foo","Bar");
    e.connect(con);

    BOOST_CHECK_EQUAL(ref, output.str());
}

BOOST_AUTO_TEST_CASE( set_max_message_size ) {
    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    
    // After the handshake, add a single frame with a message that is too long.
    char frame0[10] = {char(0x82), char(0x83), 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01};
    input.append(frame0, 10);
    
    std::string output = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\nServer: foo\r\nUpgrade: websocket\r\n\r\n";

    // After the handshake, add a single frame with a close message with message too big
    // error code.
    char frame1[4] = {char(0x88), 0x19, 0x03, char(0xf1)};
    output.append(frame1, 4);
    output.append("A message was too large");

    server s;
    s.set_user_agent("");
    s.set_validate_handler(bind(&validate_set_ua,&s,::_1));
    s.set_max_message_size(2);

    BOOST_CHECK_EQUAL(run_server_test(s,input), output);
}

BOOST_AUTO_TEST_CASE( websocket_fail_parse_error ) {
    std::string input = "asdf\r\n\r\n";

    server s;
    websocketpp::lib::error_code ec = make_error_code(websocketpp::error::http_parse_error);
    bool called = false;
    s.set_fail_handler(bind(&check_on_fail,&s,ec,websocketpp::lib::ref(called),::_1));

    run_server_test(s,input,false);
    BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE( websocket_fail_invalid_version ) {
    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: foo\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nOrigin: http://www.example.com\r\n\r\n";

    server s;
    websocketpp::lib::error_code ec = make_error_code(websocketpp::error::invalid_version);
    bool called = false;
    s.set_fail_handler(bind(&check_on_fail,&s,ec,websocketpp::lib::ref(called),::_1));

    run_server_test(s,input,false);
    BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE( websocket_fail_unsupported_version ) {
    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 12\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nOrigin: http://www.example.com\r\n\r\n";

    server s;
    websocketpp::lib::error_code ec = make_error_code(websocketpp::error::unsupported_version);
    bool called = false;
    s.set_fail_handler(bind(&check_on_fail,&s,ec,websocketpp::lib::ref(called),::_1));

    run_server_test(s,input,false);
    BOOST_CHECK(called);
}

// BOOST_AUTO_TEST_CASE( websocket_fail_invalid_uri ) {
//     std::string input = "GET http://345.123.123.123/foo HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nOrigin: http://www.example.com\r\n\r\n";

//     server s;
//     websocketpp::lib::error_code ec = make_error_code(websocketpp::error::unsupported_version);
//     bool called = false;
//     s.set_fail_handler(bind(&check_on_fail,&s,ec,websocketpp::lib::ref(called),::_1));
//     s.set_open_handler(bind(&on_open_print,&s,::_1));

//     std::cout << run_server_test(s,input,true) << std::endl;
//     BOOST_CHECK(called);
// }

// BOOST_AUTO_TEST_CASE( websocket_fail_invalid_uri_http ) {
//     std::string input = "GET http://345.123.123.123/foo HTTP/1.1\r\nHost: www.example.com\r\nOrigin: http://www.example.com\r\n\r\n";

//     server s;
//     websocketpp::lib::error_code ec = make_error_code(websocketpp::error::unsupported_version);
//     bool called = false;
//     s.set_fail_handler(bind(&check_on_fail,&s,ec,websocketpp::lib::ref(called),::_1));
//     s.set_open_handler(bind(&on_open_print,&s,::_1));

//     std::cout << run_server_test(s,input,true) << std::endl;
//     BOOST_CHECK(called);
// }

BOOST_AUTO_TEST_CASE( websocket_fail_upgrade_required ) {
    std::string input = "GET /foo/bar HTTP/1.1\r\nHost: www.example.com\r\nOrigin: http://www.example.com\r\n\r\n";

    server s;
    websocketpp::lib::error_code ec = make_error_code(websocketpp::error::upgrade_required);
    bool called = false;
    s.set_fail_handler(bind(&check_on_fail,&s,ec,websocketpp::lib::ref(called),::_1));

    run_server_test(s,input,false);
    BOOST_CHECK(called);
}

// TODO: set max message size in client endpoint test case
// TODO: set max message size mid connection test case
// TODO: [maybe] set max message size in open handler



// BOOST_AUTO_TEST_CASE( user_reject_origin ) {
//     std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nOrigin: http://www.example2.com\r\n\r\n";
//     std::string output = "HTTP/1.1 403 Forbidden\r\nServer: "+websocketpp::USER_AGENT+"\r\n\r\n";

//     BOOST_CHECK(run_server_test(input) == output);
// }

// BOOST_AUTO_TEST_CASE( basic_text_message ) {
//     std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nOrigin: http://www.example.com\r\n\r\n";

//     unsigned char frames[8] = {0x82,0x82,0xFF,0xFF,0xFF,0xFF,0xD5,0xD5};
//     input.append(reinterpret_cast<char*>(frames),8);

//     std::string output = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\nServer: "+websocketpp::USER_AGENT+"\r\nUpgrade: websocket\r\n\r\n**";

//     BOOST_CHECK( run_server_test(input) == output);
// }






BOOST_AUTO_TEST_CASE( client_handshake_timeout_race1 ) {
    debug_client c;

    websocketpp::lib::error_code ec;
    debug_client::connection_ptr con = c.get_connection("ws://localhost:9002", ec);

    BOOST_CHECK(!ec);

    // This test the case where a handshake times out immediately before the 
    // handler that would have completed it gets invoked. This situation happens
    // when clients are connecting to overloaded servers and on servers that are
    // overloaded. 
    c.connect(con);
    
    con->expire_timer(websocketpp::lib::error_code());
    // Fullfil the write to simulate the write completing immediately after
    // timer expires
    con->fullfil_write();
    
    BOOST_CHECK_EQUAL(con->get_ec(), make_error_code(websocketpp::error::open_handshake_timeout));
}

BOOST_AUTO_TEST_CASE( client_handshake_timeout_race2 ) {
    debug_client c;

    websocketpp::lib::error_code ec;
    debug_client::connection_ptr con = c.get_connection("ws://localhost:9002", ec);

    BOOST_CHECK(!ec);

    std::string output = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ICX+Yqv66kxgM0FcWaLWlFLwTAI=\r\nServer: foo\r\nUpgrade: websocket\r\n\r\n";

    // This test the case where a handshake times out immediately before the 
    // handler that would have completed it gets invoked. This situation happens
    // when clients are connecting to overloaded servers and on servers that are
    // overloaded. 
    c.connect(con);
    con->fullfil_write();
    
    con->expire_timer(websocketpp::lib::error_code());
    // Read valid handshake to simulate receiving the handshake response
    // immediately after the timer expires
    con->read_all(output.data(),output.size());
    
    BOOST_CHECK_EQUAL(con->get_ec(), make_error_code(websocketpp::error::open_handshake_timeout));
}

BOOST_AUTO_TEST_CASE( server_handshake_timeout_race1 ) {
    debug_server s;

    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: AAAAAAAAAAAAAAAAAAAAAA==\r\n\r\n";

    websocketpp::lib::error_code ec;
    debug_server::connection_ptr con = s.get_connection(ec);
    BOOST_REQUIRE(!ec);
    BOOST_REQUIRE(con);
    con->start();
    
    con->expire_timer(websocketpp::lib::error_code());
    // Read handshake immediately after timer expire
    con->read_all(input.data(), input.size());
    
    BOOST_CHECK_EQUAL(con->get_ec(), make_error_code(websocketpp::error::open_handshake_timeout));
}

BOOST_AUTO_TEST_CASE( server_handshake_timeout_race2 ) {
    debug_server s;

    std::string input = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: AAAAAAAAAAAAAAAAAAAAAA==\r\n\r\n";

    websocketpp::lib::error_code ec;
    debug_server::connection_ptr con = s.get_connection(ec);
    BOOST_REQUIRE(!ec);
    BOOST_REQUIRE(con);
    con->start();
    
    con->read_all(input.data(), input.size());
    
    con->expire_timer(websocketpp::lib::error_code());
    // Complete write immediately after timer expire
    con->fullfil_write();
    
    BOOST_CHECK_EQUAL(con->get_ec(), make_error_code(websocketpp::error::open_handshake_timeout));
}

BOOST_AUTO_TEST_CASE( queued_pong_timeout_is_ignored_after_close ) {
    server endpoint;
    endpoint.clear_access_channels(websocketpp::log::alevel::all);
    endpoint.clear_error_channels(websocketpp::log::elevel::all);
    std::stringstream output;
    endpoint.register_ostream(&output);
    int timeouts = 0;
    endpoint.set_pong_timeout_handler([&](websocketpp::connection_hdl, std::string) {
        ++timeouts;
    });
    websocketpp::lib::error_code creation_ec;
    auto con = endpoint.get_connection(creation_ec);
    BOOST_REQUIRE(!creation_ec);
    con->start();
    std::string const handshake = "GET / HTTP/1.1\r\nHost: localhost\r\n"
        "Connection: Upgrade\r\nUpgrade: websocket\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    con->read_some(handshake.data(), handshake.size());
    BOOST_REQUIRE(con->get_state() == websocketpp::session::state::open);
    con->handle_pong_timeout("live", websocketpp::lib::error_code());
    BOOST_CHECK_EQUAL(timeouts, 1);
    con->close(websocketpp::close::status::normal, "");
    // A successful completion may already be queued when cancel() is called.
    con->handle_pong_timeout("queued", websocketpp::lib::error_code());
    BOOST_CHECK_EQUAL(timeouts, 1);
    con->eof();
    con->handle_pong_timeout("closed", websocketpp::lib::error_code());
    BOOST_CHECK_EQUAL(timeouts, 1);
}

BOOST_AUTO_TEST_CASE( origin_is_available_without_websocket_processor ) {
    server endpoint;
    endpoint.clear_access_channels(websocketpp::log::alevel::all);
    endpoint.clear_error_channels(websocketpp::log::elevel::all);
    websocketpp::lib::error_code creation_ec;
    auto con = endpoint.get_connection(creation_ec);
    BOOST_REQUIRE(!creation_ec);
    BOOST_CHECK(con->get_origin().empty());
    std::stringstream output;
    con->register_ostream(&output);
    bool called = false;
    endpoint.set_http_handler([&](websocketpp::connection_hdl) {});
    con->set_http_handler([&](websocketpp::connection_hdl) {
        called = true;
        BOOST_CHECK_EQUAL(con->get_origin(), "https://example.org");
        con->set_status(websocketpp::http::status_code::ok);
    });
    con->start();
    std::string const request = "GET / HTTP/1.1\r\nHost: localhost\r\n"
        "Origin: https://example.org\r\n\r\n";
    con->read_some(request.data(), request.size());
    BOOST_CHECK(called);
    BOOST_CHECK_EQUAL(con->get_origin(), "https://example.org");
}
