// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/chrono.hpp>
#include <websocketpp/common/system_error.hpp>

#ifdef ASIO_STANDALONE
    #include <asio/version.hpp>
    #if ASIO_VERSION < 100800
        #error "The minimum version of standalone Asio is 1.8.0"
    #endif
    #include <asio.hpp>
    #include <asio/steady_timer.hpp>
#else
    #include <boost/version.hpp>
    #if BOOST_VERSION < 104900
        #error "The minimum version of Boost is 1.49.0"
    #endif
    #include <boost/asio.hpp>
    #include <boost/asio/steady_timer.hpp>
#endif

namespace websocketpp {
namespace lib {
namespace asio {

#ifdef ASIO_STANDALONE
    using namespace ::asio;
    using std::errc;
#else
    using namespace boost::asio;
    namespace errc = boost::system::errc;
#endif
    using lib::error_code;

    template <typename T>
    bool is_neg(T duration) {
        return duration.count() < 0;
    }

    inline lib::chrono::milliseconds milliseconds(long duration) {
        return lib::chrono::milliseconds(duration);
    }

} // namespace asio
} // namespace lib
} // namespace websocketpp
