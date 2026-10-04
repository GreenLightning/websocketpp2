// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_STDINT_HPP
#define WEBSOCKETPP_COMMON_STDINT_HPP

#ifndef __STDC_LIMIT_MACROS
    #define __STDC_LIMIT_MACROS 1
#endif

#if defined (_WIN32) && defined (_MSC_VER) && (_MSC_VER < 1600)
    #include <boost/cstdint.hpp>

    using boost::int8_t;
    using boost::int_least8_t;
    using boost::int_fast8_t;
    using boost::uint8_t;
    using boost::uint_least8_t;
    using boost::uint_fast8_t;

    using boost::int16_t;
    using boost::int_least16_t;
    using boost::int_fast16_t;
    using boost::uint16_t;
    using boost::uint_least16_t;
    using boost::uint_fast16_t;

    using boost::int32_t;
    using boost::int_least32_t;
    using boost::int_fast32_t;
    using boost::uint32_t;
    using boost::uint_least32_t;
    using boost::uint_fast32_t;

    #ifndef BOOST_NO_INT64_T
    using boost::int64_t;
    using boost::int_least64_t;
    using boost::int_fast64_t;
    using boost::uint64_t;
    using boost::uint_least64_t;
    using boost::uint_fast64_t;
    #endif
    using boost::intmax_t;
    using boost::uintmax_t;
#else
    #include <stdint.h>
#endif

#endif // WEBSOCKETPP_COMMON_STDINT_HPP
