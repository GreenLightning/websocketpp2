// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_CHRONO_HPP
#define WEBSOCKETPP_COMMON_CHRONO_HPP

#include <websocketpp/common/cpp11.hpp>

// If we've determined that we're in full C++11 mode and the user hasn't
// explicitly disabled the use of C++11 functional header, then prefer it to
// boost.
#if defined _WEBSOCKETPP_CPP11_INTERNAL_ && !defined _WEBSOCKETPP_NO_CPP11_CHRONO_
    #ifndef _WEBSOCKETPP_CPP11_CHRONO_
        #define _WEBSOCKETPP_CPP11_CHRONO_
    #endif
#endif

// If we're on Visual Studio 2012 or higher and haven't explicitly disabled
// the use of C++11 chrono header then prefer it to boost.
#if defined(_MSC_VER) && _MSC_VER >= 1700 && !defined _WEBSOCKETPP_NO_CPP11_CHRONO_
    #ifndef _WEBSOCKETPP_CPP11_CHRONO_
        #define _WEBSOCKETPP_CPP11_CHRONO_
    #endif
#endif

#ifdef _WEBSOCKETPP_CPP11_CHRONO_
    #include <chrono>
#else
    #include <boost/chrono.hpp>
#endif

namespace websocketpp {
namespace lib {

#ifdef _WEBSOCKETPP_CPP11_CHRONO_
    namespace chrono = std::chrono;
#else
    namespace chrono = boost::chrono;
#endif

} // namespace lib
} // namespace websocketpp

#endif // WEBSOCKETPP_COMMON_CHRONO_HPP
