// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_THREAD_HPP
#define WEBSOCKETPP_COMMON_THREAD_HPP

#include <websocketpp/common/cpp11.hpp>

#if defined(_WEBSOCKETPP_MINGW_THREAD_)
    #include <mingw-threads/mingw.thread.h>
    #include <mingw-threads/mingw.mutex.h>
    #include <mingw-threads/mingw.condition_variable.h>
#else
    #include <thread>
    #include <mutex>
    #include <condition_variable>
#endif

namespace websocketpp {
namespace lib {

using std::mutex;
using std::lock_guard;
using std::thread;
using std::unique_lock;
using std::condition_variable;

} // namespace lib
} // namespace websocketpp

#endif // WEBSOCKETPP_COMMON_THREAD_HPP
