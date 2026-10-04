// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_PLATFORMS_HPP
#define WEBSOCKETPP_COMMON_PLATFORMS_HPP

/**
 * This header contains any platform specific preprocessor adjustments that
 * don't fit somewhere else better.
 */

#if defined(_WIN32) && !defined(NOMINMAX)
    // don't define min and max macros that conflict with std::min and std::max
    #define NOMINMAX
#endif

// Bump up the variadic parameter max for Visual Studio 2012
#if defined(_MSC_VER) && _MSC_VER == 1700
    #define _VARIADIC_MAX 8
#endif

#endif // WEBSOCKETPP_COMMON_PLATFORMS_HPP
