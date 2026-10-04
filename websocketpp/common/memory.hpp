// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_MEMORY_HPP
#define WEBSOCKETPP_COMMON_MEMORY_HPP

#include <websocketpp/common/cpp11.hpp>

// If we've determined that we're in full C++11 mode and the user hasn't
// explicitly disabled the use of C++11 memory header, then prefer it to
// boost.
#if defined _WEBSOCKETPP_CPP11_INTERNAL_ && !defined _WEBSOCKETPP_NO_CPP11_MEMORY_
    #ifndef _WEBSOCKETPP_CPP11_MEMORY_
        #define _WEBSOCKETPP_CPP11_MEMORY_
    #endif
#endif

// If we're on Visual Studio 2010 or higher and haven't explicitly disabled
// the use of C++11 functional header then prefer it to boost.
#if defined(_MSC_VER) && _MSC_VER >= 1600 && !defined _WEBSOCKETPP_NO_CPP11_MEMORY_
    #ifndef _WEBSOCKETPP_CPP11_MEMORY_
        #define _WEBSOCKETPP_CPP11_MEMORY_
    #endif
#endif



#ifdef _WEBSOCKETPP_CPP11_MEMORY_
    #include <memory>
#else
    #include <boost/shared_ptr.hpp>
	#include <boost/make_shared.hpp>
    #include <boost/scoped_array.hpp>
    #include <boost/enable_shared_from_this.hpp>
    #include <boost/pointer_cast.hpp>
#endif

namespace websocketpp {
namespace lib {

#ifdef _WEBSOCKETPP_CPP11_MEMORY_
    using std::shared_ptr;
    using std::weak_ptr;
    using std::enable_shared_from_this;
    using std::static_pointer_cast;
    using std::make_shared;
    using std::unique_ptr;

    typedef std::unique_ptr<unsigned char[]> unique_ptr_uchar_array;
#else
    using boost::shared_ptr;
    using boost::weak_ptr;
    using std::auto_ptr;
    using boost::enable_shared_from_this;
    using boost::static_pointer_cast;
    using boost::make_shared;

    typedef boost::scoped_array<unsigned char> unique_ptr_uchar_array;
#endif

} // namespace lib
} // namespace websocketpp

#endif // WEBSOCKETPP_COMMON_MEMORY_HPP
