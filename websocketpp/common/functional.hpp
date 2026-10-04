// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/platforms.hpp>
#include <functional>

namespace websocketpp {
namespace lib {

using std::function;
using std::bind;
using std::ref;
namespace placeholders = std::placeholders;

// Retained for source compatibility with existing handler registrations.
#define _WEBSOCKETPP_REF(x) x

template <typename T>
void clear_function(T & x) {
    x = nullptr;
}

} // namespace lib
} // namespace websocketpp
