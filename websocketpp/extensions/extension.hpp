// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/cpp11.hpp>
#include <websocketpp/common/system_error.hpp>

#include <string>
#include <vector>

namespace websocketpp {

/**
 * Some generic information about extensions
 *
 * Each extension object has an implemented flag. It can be retrieved by calling
 * is_implemented(). This compile time flag indicates whether or not the object
 * in question actually implements the extension or if it is a placeholder stub
 *
 * Each extension object also has an enabled flag. It can be retrieved by
 * calling is_enabled(). This runtime flag indicates whether or not the
 * extension has been negotiated for this connection.
 */
namespace extensions {

namespace error {
enum value {
    /// Catch all
    general = 1,

    /// Extension disabled
    disabled
};

class category : public lib::error_category {
public:
    category() {}

    const char *name() const _WEBSOCKETPP_NOEXCEPT_TOKEN_ {
        return "websocketpp.extension";
    }

    std::string message(int value) const {
        switch(value) {
            case general:
                return "Generic extension error";
            case disabled:
                return "Use of methods from disabled extension";
            default:
                return "Unknown permessage-compress error";
        }
    }
};

inline lib::error_category const & get_category() {
    static category instance;
    return instance;
}

inline lib::error_code make_error_code(error::value e) {
    return lib::error_code(static_cast<int>(e), get_category());
}

} // namespace error
} // namespace extensions
} // namespace websocketpp

_WEBSOCKETPP_ERROR_CODE_ENUM_NS_START_
template<> struct is_error_code_enum
    <websocketpp::extensions::error::value>
{
    static const bool value = true;
};
_WEBSOCKETPP_ERROR_CODE_ENUM_NS_END_
