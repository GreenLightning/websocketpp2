// Copyright (c) 2026
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

// Including the legacy configuration header must not change backend selection.
#include <websocketpp/config/boost_config.hpp>
#include <websocketpp/common/chrono.hpp>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/common/functional.hpp>
#include <websocketpp/common/random.hpp>
#include <websocketpp/common/regex.hpp>
#include <websocketpp/common/stdint.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/type_traits.hpp>
#include <websocketpp/error.hpp>
#include <websocketpp/transport/iostream/base.hpp>

#include <iostream>
#include <type_traits>

namespace lib = websocketpp::lib;

#define CHECK_STD_TYPE(name) \
    static_assert(std::is_same<lib::name, std::name>::value, #name " must use std")

CHECK_STD_TYPE(shared_ptr<int>);
CHECK_STD_TYPE(weak_ptr<int>);
CHECK_STD_TYPE(enable_shared_from_this<int>);
CHECK_STD_TYPE(unique_ptr<int>);
CHECK_STD_TYPE(function<void()>);
CHECK_STD_TYPE(mutex);
CHECK_STD_TYPE(lock_guard<lib::mutex>);
CHECK_STD_TYPE(unique_lock<lib::mutex>);
CHECK_STD_TYPE(thread);
CHECK_STD_TYPE(condition_variable);
CHECK_STD_TYPE(chrono::steady_clock);
CHECK_STD_TYPE(random_device);
CHECK_STD_TYPE(uniform_int_distribution<int>);
CHECK_STD_TYPE(regex);
CHECK_STD_TYPE(cmatch);
static_assert(std::is_same<lib::aligned_storage<16>::type,
    std::aligned_storage<16>::type>::value, "aligned_storage must use std");
static_assert(std::is_same<lib::unique_ptr_uchar_array,
    std::unique_ptr<unsigned char[]> >::value, "array ownership must use std");
static_assert(std::is_same<websocketpp::connection_hdl,
    std::weak_ptr<void> >::value, "connection handles must use std");

#ifdef ASIO_STANDALONE
CHECK_STD_TYPE(error_code);
CHECK_STD_TYPE(error_category);
CHECK_STD_TYPE(error_condition);
CHECK_STD_TYPE(system_error);
static_assert(std::is_error_code_enum<websocketpp::error::value>::value,
    "WebSocket++ error enums must be registered with std");
#else
static_assert(std::is_same<lib::error_code, boost::system::error_code>::value,
    "Boost.Asio must use Boost.System errors");
static_assert(std::is_same<lib::error_category, boost::system::error_category>::value,
    "error categories must match error codes");
static_assert(std::is_same<lib::error_condition, boost::system::error_condition>::value,
    "error conditions must match error codes");
static_assert(std::is_same<lib::system_error, boost::system::system_error>::value,
    "system_error exceptions must match error codes");
static_assert(boost::system::is_error_code_enum<websocketpp::error::value>::value,
    "WebSocket++ error enums must be registered with Boost.System");
#endif

struct shared_object : lib::enable_shared_from_this<shared_object> {};

int main() {
    lib::shared_ptr<shared_object> object = lib::make_shared<shared_object>();
    websocketpp::connection_hdl handle = object;
    if (handle.lock() != object || object->shared_from_this() != object) {
        return 1;
    }

    int result = 0;
    lib::function<void(int)> callback = lib::bind(
        [](int & output, int value) { output = value; },
        lib::ref(result), lib::placeholders::_1);
    lib::thread worker(callback, 42);
    worker.join();
    lib::clear_function(callback);
    if (result != 42 || callback || !lib::regex_match("42", lib::regex("[0-9]+"))) {
        return 2;
    }

    lib::error_code ec = websocketpp::error::general;
    if (ec != websocketpp::error::make_error_code(websocketpp::error::general)) {
        return 3;
    }
    lib::error_code transport_ec = websocketpp::transport::iostream::error::bad_stream;
    if (!transport_ec || transport_ec.category() == ec.category()) {
        return 4;
    }
    try {
        throw lib::system_error(ec);
    } catch (lib::system_error const & e) {
        if (e.code() != ec) {
            return 5;
        }
    }
    std::cout << "C++11 aliases and backend error types passed\n";
}
