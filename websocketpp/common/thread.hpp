// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/cpp11.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>

namespace websocketpp {
namespace lib {

using std::mutex;
using std::lock_guard;
using std::thread;
using std::unique_lock;
using std::condition_variable;

} // namespace lib
} // namespace websocketpp
