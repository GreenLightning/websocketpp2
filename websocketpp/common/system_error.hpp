// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_SYSTEM_ERROR_HPP
#define WEBSOCKETPP_COMMON_SYSTEM_ERROR_HPP

#include <websocketpp/common/cpp11.hpp>

// Error codes and their categories must use the same implementation as the
// selected Asio backend, independently of the former C++11 feature defines.
#ifdef ASIO_STANDALONE
    #include <system_error>
#else
    #include <boost/system/error_code.hpp>
    #include <boost/system/system_error.hpp>
#endif

namespace websocketpp {
namespace lib {

#ifdef ASIO_STANDALONE
    using std::errc;
    using std::error_code;
    using std::error_category;
    using std::error_condition;
    using std::system_error;
    #define _WEBSOCKETPP_ERROR_CODE_ENUM_NS_START_ namespace std {
    #define _WEBSOCKETPP_ERROR_CODE_ENUM_NS_END_ }
#else
    namespace errc = boost::system::errc;
    using boost::system::error_code;
    using boost::system::error_category;
    using boost::system::error_condition;
    using boost::system::system_error;
    #define _WEBSOCKETPP_ERROR_CODE_ENUM_NS_START_ namespace boost { namespace system {
    #define _WEBSOCKETPP_ERROR_CODE_ENUM_NS_END_ }}
#endif

} // namespace lib
} // namespace websocketpp

#endif // WEBSOCKETPP_COMMON_SYSTEM_ERROR_HPP
