// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE transport_asio_tls
#include <boost/test/unit_test.hpp>

#include <websocketpp/config/asio.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>
#include <openssl/x509v3.h>
#include <memory>

#include "test_support.hpp"

BOOST_AUTO_TEST_CASE( translated_ec_tls ) {
    typedef websocketpp::transport::asio::tls_socket::connection socket_type;
    websocketpp::lib::asio::error_code ec(1, websocketpp::lib::asio::error::get_ssl_category());
    BOOST_CHECK_EQUAL(socket_type::translate_ec(ec), ec);
    BOOST_CHECK_EQUAL(socket_type::translate_ec(websocketpp::lib::asio::error_code()),
        websocketpp::lib::error_code());
}

BOOST_AUTO_TEST_CASE( server_connection_cleanup ) {
    websocketpp::server<websocketpp::config::asio_tls> s;
}

namespace {

namespace ssl = boost::asio::ssl;
typedef websocketpp::server<websocketpp::config::asio_tls> tls_server;
typedef websocketpp::client<websocketpp::config::asio_tls_client> tls_client;
typedef websocketpp::lib::shared_ptr<ssl::context> context_ptr;

struct certificate {
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> generator;
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key;
    std::unique_ptr<X509, decltype(&X509_free)> cert;

    certificate()
        : generator(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr), EVP_PKEY_CTX_free)
        , key(nullptr, EVP_PKEY_free)
        , cert(X509_new(), X509_free)
    {
        BOOST_REQUIRE(generator);
        BOOST_REQUIRE(cert);
        BOOST_REQUIRE_EQUAL(EVP_PKEY_keygen_init(generator.get()), 1);
        BOOST_REQUIRE_EQUAL(EVP_PKEY_CTX_set_rsa_keygen_bits(generator.get(), 2048), 1);
        EVP_PKEY * generated = nullptr;
        BOOST_REQUIRE_EQUAL(EVP_PKEY_keygen(generator.get(), &generated), 1);
        key.reset(generated);
        BOOST_REQUIRE_EQUAL(X509_set_version(cert.get(), 2), 1);
        BOOST_REQUIRE_EQUAL(ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1), 1);
        BOOST_REQUIRE(X509_gmtime_adj(X509_get_notBefore(cert.get()), -3600));
        BOOST_REQUIRE(X509_gmtime_adj(X509_get_notAfter(cert.get()), 86400L * 365));
        BOOST_REQUIRE_EQUAL(X509_set_pubkey(cert.get(), key.get()), 1);
        X509_NAME * subject = X509_get_subject_name(cert.get());
        BOOST_REQUIRE_EQUAL(X509_NAME_add_entry_by_txt(subject, "CN", MBSTRING_ASC,
            reinterpret_cast<unsigned char const *>("localhost"), -1, -1, 0), 1);
        BOOST_REQUIRE_EQUAL(X509_set_issuer_name(cert.get(), subject), 1);
        std::unique_ptr<X509_EXTENSION, decltype(&X509_EXTENSION_free)> san(
            X509V3_EXT_conf_nid(nullptr, nullptr, NID_subject_alt_name,
                const_cast<char *>("DNS:localhost,IP:127.0.0.1")), X509_EXTENSION_free);
        BOOST_REQUIRE(san);
        BOOST_REQUIRE_EQUAL(X509_add_ext(cert.get(), san.get(), -1), 1);
        BOOST_REQUIRE_GT(X509_sign(cert.get(), key.get(), EVP_sha256()), 0);
    }
};

struct tls_pair {
    boost::asio::io_context io;
    certificate identity;
    context_ptr server_context;
    context_ptr client_context;
    tls_server server;
    tls_client client;
    int verifications = 0;

    explicit tls_pair(bool trust_certificate)
        : server_context(new ssl::context(ssl::context::tls_server))
        , client_context(new ssl::context(ssl::context::tls_client))
    {
        BOOST_REQUIRE_EQUAL(SSL_CTX_use_certificate(server_context->native_handle(), identity.cert.get()), 1);
        BOOST_REQUIRE_EQUAL(SSL_CTX_use_PrivateKey(server_context->native_handle(), identity.key.get()), 1);
        BOOST_REQUIRE_EQUAL(SSL_CTX_check_private_key(server_context->native_handle()), 1);
        client_context->set_verify_mode(ssl::verify_peer);
        client_context->set_verify_callback([this](bool verified, ssl::verify_context & ctx) {
            ++verifications;
            if (!verified) return false;
            X509_STORE_CTX * store = ctx.native_handle();
            return X509_STORE_CTX_get_error_depth(store) != 0
                || X509_check_host(X509_STORE_CTX_get_current_cert(store),
                    "localhost", 9, 0, nullptr) == 1;
        });
        if (trust_certificate) {
            BOOST_REQUIRE_EQUAL(X509_STORE_add_cert(
                SSL_CTX_get_cert_store(client_context->native_handle()), identity.cert.get()), 1);
        }
        test_support::silence(server);
        test_support::silence(client);
        server.init_asio(&io);
        client.init_asio(&io);
        server.set_tls_init_handler([this](websocketpp::connection_hdl) { return server_context; });
        client.set_tls_init_handler([this](websocketpp::connection_hdl) { return client_context; });
        server.listen(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 0));
    }

    tls_client::connection_ptr connect() {
        websocketpp::lib::error_code ec;
        tls_client::connection_ptr con = client.get_connection("wss://127.0.0.1:"
            + std::to_string(test_support::local_endpoint(server).port()), ec);
        BOOST_REQUIRE(!ec);
        BOOST_REQUIRE(con);
        test_support::accept_one(server);
        client.connect(con);
        return con;
    }
};

} // namespace

BOOST_AUTO_TEST_CASE( tls_echo_with_certificate_verification ) {
    tls_pair pair(true);
    // Allow the default five-second TLS shutdown timeout to finish delivering
    // close callbacks after the WebSocket close handshake has completed.
    test_support::deadline deadline(pair.io, std::chrono::seconds(10));
    std::string const payload("A\0B\xff", 4);
    int opens = 0;
    int closes = 0;
    int failures = 0;
    int server_messages = 0;
    int client_messages = 0;
    pair.server.set_open_handler([&](websocketpp::connection_hdl) {
        ++opens;
    });
    pair.client.set_open_handler([&](websocketpp::connection_hdl hdl) {
        ++opens;
        websocketpp::lib::error_code ec;
        pair.client.send(hdl, payload, websocketpp::frame::opcode::BINARY, ec);
        BOOST_CHECK(!ec);
    });
    pair.server.set_message_handler([&](websocketpp::connection_hdl hdl, tls_server::message_ptr msg) {
        ++server_messages;
        BOOST_CHECK_EQUAL(msg->get_payload(), payload);
        BOOST_CHECK_EQUAL(msg->get_opcode(), websocketpp::frame::opcode::BINARY);
        websocketpp::lib::error_code ec;
        pair.server.send(hdl, msg, ec);
        BOOST_CHECK(!ec);
    });
    pair.client.set_message_handler([&](websocketpp::connection_hdl hdl, tls_client::message_ptr msg) {
        ++client_messages;
        BOOST_CHECK_EQUAL(msg->get_payload(), payload);
        BOOST_CHECK_EQUAL(msg->get_opcode(), websocketpp::frame::opcode::BINARY);
        pair.client.close(hdl, websocketpp::close::status::normal, "done");
    });
    auto closed = [&](websocketpp::connection_hdl) { if (++closes == 2) deadline.cancel(); };
    pair.server.set_close_handler(closed);
    pair.client.set_close_handler(closed);
    auto failed = [&](websocketpp::connection_hdl) { ++failures; pair.io.stop(); };
    pair.server.set_fail_handler(failed);
    pair.client.set_fail_handler(failed);
    tls_client::connection_ptr con = pair.connect();
    pair.io.run();
    BOOST_CHECK(!deadline.expired());
    BOOST_CHECK_GT(pair.verifications, 0);
    BOOST_CHECK_EQUAL(opens, 2);
    BOOST_CHECK_EQUAL(closes, 2);
    BOOST_CHECK_EQUAL(failures, 0);
    BOOST_CHECK_EQUAL(server_messages, 1);
    BOOST_CHECK_EQUAL(client_messages, 1);
    BOOST_CHECK(!con->get_ec());
    BOOST_CHECK_EQUAL(con->get_local_close_code(), websocketpp::close::status::normal);
    BOOST_CHECK_EQUAL(con->get_remote_close_code(), websocketpp::close::status::normal);
    BOOST_CHECK(con->get_state() == websocketpp::session::state::closed);
}

BOOST_AUTO_TEST_CASE( tls_rejects_untrusted_certificate ) {
    tls_pair pair(false);
    test_support::deadline deadline(pair.io);
    int opens = 0;
    int messages = 0;
    int client_failures = 0;
    int server_failures = 0;
    auto opened = [&](websocketpp::connection_hdl) { ++opens; };
    pair.server.set_open_handler(opened);
    pair.client.set_open_handler(opened);
    pair.client.set_message_handler([&](websocketpp::connection_hdl, tls_client::message_ptr) { ++messages; });
    auto finish = [&] { if (client_failures && server_failures) deadline.cancel(); };
    pair.client.set_fail_handler([&](websocketpp::connection_hdl) { ++client_failures; finish(); });
    pair.server.set_fail_handler([&](websocketpp::connection_hdl) {
        ++server_failures;
        finish();
    });
    tls_client::connection_ptr con = pair.connect();
    pair.io.run();
    BOOST_CHECK(!deadline.expired());
    BOOST_CHECK_GT(pair.verifications, 0);
    BOOST_CHECK_EQUAL(client_failures, 1);
    BOOST_CHECK_EQUAL(server_failures, 1);
    BOOST_CHECK_EQUAL(opens, 0);
    BOOST_CHECK_EQUAL(messages, 0);
    BOOST_CHECK_EQUAL(con->get_ec(), websocketpp::transport::asio::socket::make_error_code(
        websocketpp::transport::asio::socket::error::tls_handshake_failed));
    BOOST_CHECK(con->get_state() == websocketpp::session::state::closed);
}

BOOST_AUTO_TEST_CASE( missing_tls_initialization_handler ) {
    struct socket : websocketpp::transport::asio::tls_socket::connection {
        using websocketpp::transport::asio::tls_socket::connection::init_asio;
    };
    boost::asio::io_context io;
    socket connection;
    BOOST_CHECK_EQUAL(connection.init_asio(&io, socket::strand_ptr(), false),
        websocketpp::transport::asio::socket::make_error_code(
            websocketpp::transport::asio::socket::error::missing_tls_init_handler));
    BOOST_CHECK_EQUAL(io.poll(), 0u);
}
