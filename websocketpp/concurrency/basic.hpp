// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_CONCURRENCY_BASIC_HPP
#define WEBSOCKETPP_CONCURRENCY_BASIC_HPP

#include <websocketpp/common/thread.hpp>

namespace websocketpp {
namespace concurrency {

/// Concurrency policy that uses std::mutex
class basic {
public:
    typedef lib::mutex mutex_type;
    typedef lib::lock_guard<mutex_type> scoped_lock_type;
};

} // namespace concurrency
} // namespace websocketpp

#endif // WEBSOCKETPP_CONCURRENCY_BASIC_HPP
