// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_FUNCTIONAL_HPP
#define WEBSOCKETPP_COMMON_FUNCTIONAL_HPP

#include <websocketpp/common/cpp11.hpp>

// If we've determined that we're in full C++11 mode and the user hasn't
// explicitly disabled the use of C++11 functional header, then prefer it to
// boost.
#if defined _WEBSOCKETPP_CPP11_INTERNAL_ && !defined _WEBSOCKETPP_NO_CPP11_FUNCTIONAL_
    #ifndef _WEBSOCKETPP_CPP11_FUNCTIONAL_
        #define _WEBSOCKETPP_CPP11_FUNCTIONAL_
    #endif
#endif

// If we're on Visual Studio 2010 or higher and haven't explicitly disabled
// the use of C++11 functional header then prefer it to boost.
#if defined(_MSC_VER) && _MSC_VER >= 1600 && !defined _WEBSOCKETPP_NO_CPP11_FUNCTIONAL_
    #ifndef _WEBSOCKETPP_CPP11_FUNCTIONAL_
        #define _WEBSOCKETPP_CPP11_FUNCTIONAL_
    #endif
#endif



#ifdef _WEBSOCKETPP_CPP11_FUNCTIONAL_
    #include <functional>
#else
    #include <boost/bind.hpp>
    #include <boost/function.hpp>
    #include <boost/ref.hpp>
#endif



namespace websocketpp {
namespace lib {

#ifdef _WEBSOCKETPP_CPP11_FUNCTIONAL_
    using std::function;
    using std::bind;
    using std::ref;
    namespace placeholders = std::placeholders;

    // There are some cases where a C++11 compiler balks at using std::ref
    // but a C++03 compiler using boost function requires boost::ref. As such
    // lib::ref is not useful in these cases. Instead this macro allows the use
    // of boost::ref in the case of a boost compile or no reference wrapper at
    // all in the case of a C++11 compile
    #define _WEBSOCKETPP_REF(x) x

    template <typename T>
    void clear_function(T & x) {
        x = nullptr;
    }
#else
    using boost::function;
    using boost::bind;
    using boost::ref;
    namespace placeholders {
        /// \todo this feels hacky, is there a better way?
        using ::_1;
        using ::_2;
        using ::_3;
    }

    // See above definition for more details on what this is and why it exists
    #define _WEBSOCKETPP_REF(x) boost::ref(x)

    template <typename T>
    void clear_function(T & x) {
        x.clear();
    }
#endif

} // namespace lib
} // namespace websocketpp

#endif // WEBSOCKETPP_COMMON_FUNCTIONAL_HPP
