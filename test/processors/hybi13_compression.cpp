// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE hybi_13_compression
#include <boost/test/unit_test.hpp>

#include "hybi13_test.hpp"
#include <websocketpp/extensions/permessage_deflate/enabled.hpp>

struct stub_config_ext {
    typedef websocketpp::http::parser::request request_type;
    typedef websocketpp::http::parser::response response_type;

    typedef websocketpp::message_buffer::message
        <websocketpp::message_buffer::alloc::con_msg_manager> message_type;
    typedef websocketpp::message_buffer::alloc::con_msg_manager<message_type>
        con_msg_manager_type;

    typedef websocketpp::random::none::int_generator<uint32_t> rng_type;

    struct permessage_deflate_config {
        typedef stub_config_ext::request_type request_type;
        static const size_t max_message_size = 16000000;
    };

    typedef websocketpp::extensions::permessage_deflate::enabled
        <permessage_deflate_config> permessage_deflate_type;

    static const size_t max_message_size = 16000000;
    static const bool enable_extensions = true;
};

struct processor_setup_ext {
    processor_setup_ext(bool server)
      : msg_manager(new con_msg_manager_type())
      , p(false,server,msg_manager,rng) {}

    websocketpp::lib::error_code ec;
    con_msg_manager_type::ptr msg_manager;
    stub_config::rng_type rng;
    stub_config::request_type req;
    stub_config::response_type res;
    websocketpp::processor::hybi13<stub_config_ext> p;
};

BOOST_AUTO_TEST_CASE( compressed_frame_message_too_large ) {
    processor_setup_ext client(false);
    processor_setup_ext server(true);
    size_t const limit = stub_config_ext::max_message_size;

    std::pair<websocketpp::lib::error_code,std::string> neg_results;

    server.req.replace_header(
        "Sec-WebSocket-Extensions",
        "permessage-deflate; client_max_window_bits"
    );

    neg_results = server.p.negotiate_extensions(server.req);
    BOOST_REQUIRE( !neg_results.first );

    client.res.replace_header("Sec-WebSocket-Extensions",neg_results.second);
    neg_results = client.p.negotiate_extensions(client.res);
    BOOST_REQUIRE( !neg_results.first );

    message_ptr in = client.msg_manager->get_message();
    message_ptr out = client.msg_manager->get_message();

    BOOST_REQUIRE( in );
    BOOST_REQUIRE( out );

    in->set_opcode(websocketpp::frame::opcode::BINARY);
    in->set_payload(std::string(limit + 1, '*'));
    in->set_compressed(true);

    client.ec = client.p.prepare_data_frame(in,out);
    BOOST_REQUIRE_EQUAL( client.ec, websocketpp::lib::error_code() );

    std::string frame = out->get_header();
    frame += out->get_payload();
    std::vector<uint8_t> frame_bytes(frame.begin(),frame.end());

    BOOST_CHECK_GT( server.p.consume(&frame_bytes[0],frame_bytes.size(),server.ec), 0 );
    BOOST_CHECK_EQUAL( server.ec, websocketpp::processor::error::message_too_big );
    BOOST_CHECK_EQUAL( server.p.ready(), false );
    BOOST_CHECK_EQUAL( server.p.get_message(), message_ptr() );
}

// Negotiate permessage-deflate between a client and server processor pair
// using default options. Returns true on success.
static bool negotiate_permessage_deflate(processor_setup_ext & client,
                                         processor_setup_ext & server)
{
    std::pair<websocketpp::lib::error_code,std::string> neg;

    server.req.replace_header(
        "Sec-WebSocket-Extensions",
        "permessage-deflate; client_max_window_bits"
    );
    neg = server.p.negotiate_extensions(server.req);
    if (neg.first) return false;

    client.res.replace_header("Sec-WebSocket-Extensions", neg.second);
    neg = client.p.negotiate_extensions(client.res);
    return !neg.first;
}


// Verifies that lowering the limit at runtime via set_max_message_size on
// the processor reaches the permessage-deflate extension. Without runtime
// propagation through the handle_max_message_size_changed hook, the
// extension would still use its construction-time limit and the oversized
// payload below would decompress without error.
BOOST_AUTO_TEST_CASE( compressed_frame_runtime_lower_limit_propagates ) {
    processor_setup_ext client(false);
    processor_setup_ext server(true);

    BOOST_REQUIRE( negotiate_permessage_deflate(client, server) );

    size_t const runtime_limit = 1024;
    server.p.set_max_message_size(runtime_limit);

    // Payload is well above the runtime limit but well below the test
    // config's max_message_size. Without propagation, this would decompress
    // successfully under the original 16M limit.
    message_ptr in = client.msg_manager->get_message();
    message_ptr out = client.msg_manager->get_message();
    BOOST_REQUIRE( in );
    BOOST_REQUIRE( out );

    in->set_opcode(websocketpp::frame::opcode::BINARY);
    in->set_payload(std::string(runtime_limit * 4, '*'));
    in->set_compressed(true);

    client.ec = client.p.prepare_data_frame(in, out);
    BOOST_REQUIRE_EQUAL( client.ec, websocketpp::lib::error_code() );

    std::string frame = out->get_header();
    frame += out->get_payload();
    std::vector<uint8_t> frame_bytes(frame.begin(), frame.end());

    BOOST_CHECK_GT( server.p.consume(&frame_bytes[0], frame_bytes.size(), server.ec), 0 );
    BOOST_CHECK_EQUAL( server.ec, websocketpp::processor::error::message_too_big );
    BOOST_CHECK_EQUAL( server.p.ready(), false );
    BOOST_CHECK_EQUAL( server.p.get_message(), message_ptr() );
}

// Verifies that subsequent set_max_message_size calls also propagate, not
// just the first. After lowering then raising the limit, a payload that
// would have failed under the lower limit should now succeed.
BOOST_AUTO_TEST_CASE( compressed_frame_runtime_raise_limit_propagates ) {
    processor_setup_ext client(false);
    processor_setup_ext server(true);

    BOOST_REQUIRE( negotiate_permessage_deflate(client, server) );

    server.p.set_max_message_size(1024);
    server.p.set_max_message_size(8192);

    message_ptr in = client.msg_manager->get_message();
    message_ptr out = client.msg_manager->get_message();
    BOOST_REQUIRE( in );
    BOOST_REQUIRE( out );

    // 4096 bytes: would fail under the first (1024) limit, must succeed
    // under the second (8192) limit if the second call propagated.
    std::string const payload(4096, '*');
    in->set_opcode(websocketpp::frame::opcode::BINARY);
    in->set_payload(payload);
    in->set_compressed(true);

    client.ec = client.p.prepare_data_frame(in, out);
    BOOST_REQUIRE_EQUAL( client.ec, websocketpp::lib::error_code() );

    std::string frame = out->get_header();
    frame += out->get_payload();
    std::vector<uint8_t> frame_bytes(frame.begin(), frame.end());

    server.p.consume(&frame_bytes[0], frame_bytes.size(), server.ec);
    BOOST_CHECK_EQUAL( server.ec, websocketpp::lib::error_code() );
    BOOST_CHECK_EQUAL( server.p.ready(), true );
    message_ptr received = server.p.get_message();
    BOOST_REQUIRE( received );
    BOOST_CHECK_EQUAL( received->get_payload(), payload );
}

// Verifies that the limit is enforced when a compressed message is split
// across multiple WebSocket frames. The single-frame test exercises the
// in-decompress overflow path on the first call; this test exercises the
// state of the limit check across multiple consume_payload_chunk
// invocations with `out` accumulating between fragments.
BOOST_AUTO_TEST_CASE( fragmented_compressed_message_too_large ) {
    processor_setup_ext client(false);
    processor_setup_ext server(true);

    BOOST_REQUIRE( negotiate_permessage_deflate(client, server) );

    size_t const runtime_limit = 512;
    server.p.set_max_message_size(runtime_limit);

    // Produce a compressed payload that decompresses to several times
    // the limit. Compression of repeated bytes is dense, so the resulting
    // compressed payload is small enough that the per-frame size check
    // (which compares decompressed `out` size against compressed frame
    // payload size) will not pre-emptively trigger.
    message_ptr in = client.msg_manager->get_message();
    message_ptr out = client.msg_manager->get_message();
    BOOST_REQUIRE( in );
    BOOST_REQUIRE( out );

    in->set_opcode(websocketpp::frame::opcode::BINARY);
    in->set_payload(std::string(runtime_limit * 4, '*'));
    in->set_compressed(true);

    client.ec = client.p.prepare_data_frame(in, out);
    BOOST_REQUIRE_EQUAL( client.ec, websocketpp::lib::error_code() );

    // Re-frame the compressed payload across three masked WebSocket frames:
    // (FIN=0, RSV1=1, BINARY), (FIN=0, CONT), (FIN=1, CONT).
    std::string const & compressed = out->get_payload();
    BOOST_REQUIRE_GE( compressed.size(), 3u );

    size_t const slice = compressed.size() / 3;
    std::string c0 = compressed.substr(0, slice);
    std::string c1 = compressed.substr(slice, slice);
    std::string c2 = compressed.substr(slice * 2);

    std::vector<uint8_t> f0 = build_masked_frame(false, true,  0x02, c0);
    std::vector<uint8_t> f1 = build_masked_frame(false, false, 0x00, c1);
    std::vector<uint8_t> f2 = build_masked_frame(true,  false, 0x00, c2);

    std::vector<uint8_t> stream;
    stream.insert(stream.end(), f0.begin(), f0.end());
    stream.insert(stream.end(), f1.begin(), f1.end());
    stream.insert(stream.end(), f2.begin(), f2.end());

    server.p.consume(&stream[0], stream.size(), server.ec);
    BOOST_CHECK_EQUAL( server.ec, websocketpp::processor::error::message_too_big );
    BOOST_CHECK_EQUAL( server.p.ready(), false );
    BOOST_CHECK_EQUAL( server.p.get_message(), message_ptr() );
}




BOOST_AUTO_TEST_CASE( extension_negotiation_blank ) {
    processor_setup_ext env(true);

    env.req.replace_header("Sec-WebSocket-Extensions","");

    std::pair<websocketpp::lib::error_code,std::string> neg_results;
    neg_results = env.p.negotiate_extensions(env.req);

    BOOST_CHECK( !neg_results.first );
    BOOST_CHECK_EQUAL( neg_results.second, "" );
}

BOOST_AUTO_TEST_CASE( extension_negotiation_unknown ) {
    processor_setup_ext env(true);

    env.req.replace_header("Sec-WebSocket-Extensions","foo");

    std::pair<websocketpp::lib::error_code,std::string> neg_results;
    neg_results = env.p.negotiate_extensions(env.req);

    BOOST_CHECK( !neg_results.first );
    BOOST_CHECK_EQUAL( neg_results.second, "" );
}


BOOST_AUTO_TEST_CASE( extension_negotiation_permessage_deflate ) {
    processor_setup_ext env(true);

    env.req.replace_header("Sec-WebSocket-Extensions",
        "permessage-deflate; client_max_window_bits");

    std::pair<websocketpp::lib::error_code,std::string> neg_results;
    neg_results = env.p.negotiate_extensions(env.req);

    BOOST_CHECK( !neg_results.first );
    BOOST_CHECK_EQUAL( neg_results.second, "permessage-deflate" );
}
