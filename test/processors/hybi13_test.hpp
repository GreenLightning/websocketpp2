// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_TEST_PROCESSORS_HYBI13_TEST_HPP
#define WEBSOCKETPP_TEST_PROCESSORS_HYBI13_TEST_HPP

#include <iostream>
#include <string>
#include <vector>

#include <websocketpp/processors/hybi13.hpp>

#include <websocketpp/http/request.hpp>
#include <websocketpp/http/response.hpp>
#include <websocketpp/message_buffer/message.hpp>
#include <websocketpp/message_buffer/alloc.hpp>
#include <websocketpp/random/none.hpp>

#include <websocketpp/extensions/permessage_deflate/disabled.hpp>

struct stub_config {
    typedef websocketpp::http::parser::request request_type;
    typedef websocketpp::http::parser::response response_type;

    typedef websocketpp::message_buffer::message
        <websocketpp::message_buffer::alloc::con_msg_manager> message_type;
    typedef websocketpp::message_buffer::alloc::con_msg_manager<message_type>
        con_msg_manager_type;

    typedef websocketpp::random::none::int_generator<uint32_t> rng_type;

    struct permessage_deflate_config {
        typedef stub_config::request_type request_type;
        static const size_t max_message_size = 16000000;
    };

    typedef websocketpp::extensions::permessage_deflate::disabled
        <permessage_deflate_config> permessage_deflate_type;

    static const size_t max_message_size = 16000000;
    static const bool enable_extensions = false;
};

typedef stub_config::con_msg_manager_type con_msg_manager_type;
typedef stub_config::message_type::ptr message_ptr;

// Set up a structure that constructs new copies of all of the support structure
// for using connection processors
struct processor_setup {
    processor_setup(bool server)
      : msg_manager(new con_msg_manager_type())
      , p(false,server,msg_manager,rng) {}

    websocketpp::lib::error_code ec;
    con_msg_manager_type::ptr msg_manager;
    stub_config::rng_type rng;
    stub_config::request_type req;
    stub_config::response_type res;
    websocketpp::processor::hybi13<stub_config> p;
};

// Build a masked WebSocket frame with a zero mask key. Suitable for feeding
// to a server processor; the zero mask leaves the payload bytes unchanged
// after unmask, which lets us compose deflate output across multiple frames
// without needing to track a real RNG-generated mask per fragment.
inline std::vector<uint8_t> build_masked_frame(bool fin, bool rsv1,
                                                uint8_t opcode,
                                                std::string const & payload)
{
    std::vector<uint8_t> frame;
    uint8_t b0 = static_cast<uint8_t>(
        (fin ? 0x80 : 0x00) | (rsv1 ? 0x40 : 0x00) | (opcode & 0x0F)
    );
    frame.push_back(b0);

    size_t const len = payload.size();
    if (len < 126) {
        frame.push_back(static_cast<uint8_t>(0x80 | len));
    } else if (len <= 0xFFFF) {
        frame.push_back(static_cast<uint8_t>(0x80 | 126));
        frame.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        frame.push_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        frame.push_back(static_cast<uint8_t>(0x80 | 127));
        for (int i = 7; i >= 0; --i) {
            frame.push_back(static_cast<uint8_t>((len >> (i * 8)) & 0xFF));
        }
    }

    // Zero mask key (4 bytes). XOR with 0 leaves payload unchanged.
    frame.push_back(0x00);
    frame.push_back(0x00);
    frame.push_back(0x00);
    frame.push_back(0x00);

    frame.insert(frame.end(), payload.begin(), payload.end());
    return frame;
}

#endif // WEBSOCKETPP_TEST_PROCESSORS_HYBI13_TEST_HPP
