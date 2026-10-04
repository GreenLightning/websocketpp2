// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_TIME_HPP
#define WEBSOCKETPP_COMMON_TIME_HPP

#include <ctime>

namespace websocketpp {
namespace lib {

// Code in this header was inspired by the following article and includes some
// code from the related project g2log. The g2log code is public domain licensed
// http://kjellkod.wordpress.com/2013/01/22/exploring-c11-part-2-localtime-and-time-again/

/// Thread safe cross platform localtime
inline std::tm localtime(std::time_t const & time) {
    std::tm tm_snapshot;
#if (defined(__MINGW32__) || defined(__MINGW64__))
    memcpy(&tm_snapshot, ::localtime(&time), sizeof(std::tm));
#elif (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
    localtime_s(&tm_snapshot, &time); 
#else
    localtime_r(&time, &tm_snapshot); // POSIX  
#endif
    return tm_snapshot;
}

} // lib
} // websocketpp

#endif // WEBSOCKETPP_COMMON_TIME_HPP
