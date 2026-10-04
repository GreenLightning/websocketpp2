// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

// MSVC historically reports C++98 in __cplusplus unless /Zc:__cplusplus
// is enabled. VS 2015 and later provide the C++11 features used here.
#if defined(_MSVC_LANG)
    #if _MSVC_LANG < 201103L
        #error "WebSocket++ requires C++11 or later"
    #endif
#elif defined(_MSC_VER)
    #if _MSC_VER < 1900
        #error "WebSocket++ requires C++11 or later"
    #endif
#elif __cplusplus < 201103L
    #error "WebSocket++ requires C++11 or later"
#endif

// Retain the feature and token macros for source compatibility. The C++11
// language and standard library features are now always available.
#ifndef _WEBSOCKETPP_CPP11_INTERNAL_
    #define _WEBSOCKETPP_CPP11_INTERNAL_
#endif
#ifndef _WEBSOCKETPP_NOEXCEPT_TOKEN_
    #define _WEBSOCKETPP_NOEXCEPT_TOKEN_ noexcept
#endif
#ifndef _WEBSOCKETPP_CONSTEXPR_TOKEN_
    #define _WEBSOCKETPP_CONSTEXPR_TOKEN_ constexpr
#endif
#ifndef _WEBSOCKETPP_NULLPTR_TOKEN_
    #define _WEBSOCKETPP_NULLPTR_TOKEN_ nullptr
#endif
#ifndef _WEBSOCKETPP_INITIALIZER_LISTS_
    #define _WEBSOCKETPP_INITIALIZER_LISTS_
#endif
#ifndef _WEBSOCKETPP_MOVE_SEMANTICS_
    #define _WEBSOCKETPP_MOVE_SEMANTICS_
#endif
#ifndef _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_
    #define _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_
#endif
#ifndef _WEBSOCKETPP_PUTTIME_
    #define _WEBSOCKETPP_PUTTIME_
#endif
