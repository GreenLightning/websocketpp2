// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_COMMON_MEMORY_HPP
#define WEBSOCKETPP_COMMON_MEMORY_HPP

#include <websocketpp/common/cpp11.hpp>
#include <memory>

namespace websocketpp {
namespace lib {

using std::shared_ptr;
using std::weak_ptr;
using std::enable_shared_from_this;
using std::static_pointer_cast;
using std::make_shared;
using std::unique_ptr;

typedef std::unique_ptr<unsigned char[]> unique_ptr_uchar_array;

} // namespace lib
} // namespace websocketpp

#endif // WEBSOCKETPP_COMMON_MEMORY_HPP
