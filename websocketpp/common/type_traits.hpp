// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_TYPE_TRAITS_HPP
#define WEBSOCKETPP_COMMON_TYPE_TRAITS_HPP

#include <websocketpp/common/cpp11.hpp>

// If we've determined that we're in full C++11 mode and the user hasn't
// explicitly disabled the use of C++11 functional header, then prefer it to
// boost.
#if defined _WEBSOCKETPP_CPP11_INTERNAL_ && !defined _WEBSOCKETPP_NO_CPP11_TYPE_TRAITS_
    #ifndef _WEBSOCKETPP_CPP11_TYPE_TRAITS_
        #define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
    #endif
#endif


#ifdef _WEBSOCKETPP_CPP11_TYPE_TRAITS_
    #include <type_traits>
#else
    #include <boost/aligned_storage.hpp>
#endif



namespace websocketpp {
namespace lib {

#ifdef _WEBSOCKETPP_CPP11_TYPE_TRAITS_
    using std::aligned_storage;
    using std::is_same;
#else
    using boost::aligned_storage;
    using boost::is_same;
#endif

} // namespace lib
} // namespace websocketpp

#endif // WEBSOCKETPP_COMMON_TYPE_TRAITS_HPP
