// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

/**
 * This header checks compiler requirements and contains platform specific
 * preprocessor adjustments that don't fit somewhere else better.
 */

// Enforce the minimum language standard for header-only consumers.
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

#if defined(_WIN32) && !defined(NOMINMAX)
    // don't define min and max macros that conflict with std::min and std::max
    #define NOMINMAX
#endif

// C++11 has no standard deprecation attribute. Use compiler annotations so
// deprecated APIs warn without requiring C++14 or compiler extensions in users.
#if defined(_MSC_VER)
    #define _WEBSOCKETPP_DEPRECATED_(message) __declspec(deprecated(message))
#elif defined(__GNUC__) || defined(__clang__)
    #define _WEBSOCKETPP_DEPRECATED_(message) __attribute__((deprecated(message)))
#else
    #define _WEBSOCKETPP_DEPRECATED_(message)
#endif
