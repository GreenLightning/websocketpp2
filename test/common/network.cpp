// Copyright (c) 2026, Green Lightning
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#define BOOST_TEST_MODULE common_network
#include <boost/test/unit_test.hpp>

#include <websocketpp/common/network.hpp>

#include <atomic>
#include <cstring>
#include <thread>
#include <vector>

BOOST_AUTO_TEST_CASE(concurrent_first_conversion) {
    const unsigned thread_count = 8;
    std::atomic<unsigned> ready(0);
    std::atomic<bool> start(false);
    std::atomic<unsigned> failures(0);
    std::vector<std::thread> workers;

    // No byte-order conversion precedes these calls: exercise simultaneous
    // initialization as well as concurrent conversions under ThreadSanitizer.
    for (unsigned t = 0; t < thread_count; ++t) {
        workers.emplace_back([&] {
            const uint64_t values[] = {
                0, 1, 0xff, 0x100, 0xffffffffULL, 0x100000000ULL,
                0x0123456789abcdefULL, 0xffffffffffffffffULL
            };
            ++ready;
            while (!start.load()) { std::this_thread::yield(); }
            for (unsigned i = 0; i < 4096; ++i) {
                const uint64_t value = values[i % 8];
                unsigned char expected[8];
                for (unsigned j = 0; j < 8; ++j) {
                    expected[j] = static_cast<unsigned char>(value >> (56 - 8 * j));
                }
                const uint64_t network = websocketpp::lib::net::_htonll(value);
                if (std::memcmp(&network, expected, sizeof(network)) != 0 ||
                    websocketpp::lib::net::_ntohll(network) != value) {
                    ++failures;
                }
            }
        });
    }
    while (ready.load() != thread_count) { std::this_thread::yield(); }
    start = true;
    for (auto & worker : workers) { worker.join(); }
    BOOST_CHECK_EQUAL(failures.load(), 0);
}
