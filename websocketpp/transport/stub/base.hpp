// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/system_error.hpp>
#include <websocketpp/common/platforms.hpp>

#include <string>

namespace websocketpp {
namespace transport {
/// Stub transport policy that has no input or output.
namespace stub {

/// stub transport errors
namespace error {
enum value {
    /// Catch-all error for transport policy errors that don't fit in other
    /// categories
    general = 1,

    /// not implemented
    not_implemented
};

/// stub transport error category
class category : public lib::error_category {
    public:
    category() {}

    char const * name() const noexcept {
        return "websocketpp.transport.stub";
    }

    std::string message(int value) const {
        switch(value) {
            case general:
                return "Generic stub transport policy error";
            case not_implemented:
                return "feature not implemented";
            default:
                return "Unknown";
        }
    }
};

/// Get a reference to a static copy of the stub transport error category
inline lib::error_category const & get_category() {
    static category instance;
    return instance;
}

/// Get an error code with the given value and the stub transport category
inline lib::error_code make_error_code(error::value e) {
    return lib::error_code(static_cast<int>(e), get_category());
}

} // namespace error
} // namespace stub
} // namespace transport
} // namespace websocketpp
_WEBSOCKETPP_ERROR_CODE_ENUM_NS_START_
template<> struct is_error_code_enum<websocketpp::transport::stub::error::value>
{
    static bool const value = true;
};
_WEBSOCKETPP_ERROR_CODE_ENUM_NS_END_
