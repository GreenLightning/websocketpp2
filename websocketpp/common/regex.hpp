// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_REGEX_HPP
#define WEBSOCKETPP_COMMON_REGEX_HPP

#if defined _WEBSOCKETPP_CPP11_STL_ && !defined _WEBSOCKETPP_NO_CPP11_REGEX_
    #ifndef _WEBSOCKETPP_CPP11_REGEX_
        #define _WEBSOCKETPP_CPP11_REGEX_
    #endif
#endif

#ifdef _WEBSOCKETPP_CPP11_REGEX_
    #include <regex>
#else
    #include <boost/regex.hpp>
#endif

namespace websocketpp {
namespace lib {

#ifdef _WEBSOCKETPP_CPP11_REGEX_
    using std::cmatch;
    using std::regex;
    using std::regex_match;
#else
    using boost::cmatch;
    using boost::regex;
    using boost::regex_match;
#endif

} // namespace lib
} // namespace websocketpp

#endif // WEBSOCKETPP_COMMON_REGEX_HPP
