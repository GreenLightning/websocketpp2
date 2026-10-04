// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

 // This header defines WebSocket++ macros for C++11 compatibility based on the 
 // Boost.Config library. This will correctly configure most target platforms
 // simply by including this header before any other WebSocket++ header.

#ifndef WEBSOCKETPP_CONFIG_BOOST_CONFIG_HPP
#define WEBSOCKETPP_CONFIG_BOOST_CONFIG_HPP

#include <boost/config.hpp>

//  _WEBSOCKETPP_CPP11_MEMORY_ and _WEBSOCKETPP_CPP11_FUNCTIONAL_ presently
//  only work if either both or neither is defined.
#if !defined BOOST_NO_CXX11_SMART_PTR && !defined BOOST_NO_CXX11_HDR_FUNCTIONAL
    #define _WEBSOCKETPP_CPP11_MEMORY_
    #define _WEBSOCKETPP_CPP11_FUNCTIONAL_
#endif

#ifdef BOOST_ASIO_HAS_STD_CHRONO
    #define _WEBSOCKETPP_CPP11_CHRONO_
#endif

#ifndef BOOST_NO_CXX11_HDR_RANDOM
    #define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#endif

#ifndef BOOST_NO_CXX11_HDR_REGEX
    #define _WEBSOCKETPP_CPP11_REGEX_
#endif

#ifndef BOOST_NO_CXX11_HDR_SYSTEM_ERROR
    #define _WEBSOCKETPP_CPP11_SYSTEM_ERROR_
#endif

#ifndef BOOST_NO_CXX11_HDR_THREAD
    #define _WEBSOCKETPP_CPP11_THREAD_
#endif

#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
    #define _WEBSOCKETPP_INITIALIZER_LISTS_
#endif

#define _WEBSOCKETPP_NOEXCEPT_TOKEN_  BOOST_NOEXCEPT
#define _WEBSOCKETPP_CONSTEXPR_TOKEN_  BOOST_CONSTEXPR
// TODO: nullptr support

#endif // WEBSOCKETPP_CONFIG_BOOST_CONFIG_HPP
