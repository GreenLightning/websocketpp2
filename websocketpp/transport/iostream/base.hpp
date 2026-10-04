// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/system_error.hpp>
#include <websocketpp/common/platforms.hpp>
#include <websocketpp/common/functional.hpp>
#include <websocketpp/common/connection_hdl.hpp>

#include <websocketpp/transport/base/connection.hpp>

#include <string>
#include <vector>

namespace websocketpp {
namespace transport {
/// Transport policy that uses STL iostream for I/O and does not support timers
namespace iostream {

/// The type and signature of the callback used by iostream transport to write
typedef lib::function<lib::error_code(connection_hdl, char const *, size_t)>
    write_handler;

/// The type and signature of the callback used by iostream transport to perform
/// vectored writes.
/**
 * If a vectored write handler is not set the standard write handler will be
 * called multiple times.
 */
typedef lib::function<lib::error_code(connection_hdl, std::vector<transport::buffer> const
    & bufs)> vector_write_handler;

/// The type and signature of the callback used by iostream transport to signal
/// a transport shutdown.
typedef lib::function<lib::error_code(connection_hdl)> shutdown_handler;

/// iostream transport errors
namespace error {
enum value {
    /// Catch-all error for transport policy errors that don't fit in other
    /// categories
    general = 1,

    /// async_read_at_least call requested more bytes than buffer can store
    invalid_num_bytes,

    /// async_read called while another async_read was in progress
    double_read,

    /// An operation that requires an output stream was attempted before
    /// setting one.
    output_stream_required,

    /// stream error
    bad_stream
};

/// iostream transport error category
class category : public lib::error_category {
    public:
    category() {}

    char const * name() const noexcept {
        return "websocketpp.transport.iostream";
    }

    std::string message(int value) const {
        switch(value) {
            case general:
                return "Generic iostream transport policy error";
            case invalid_num_bytes:
                return "async_read_at_least call requested more bytes than buffer can store";
            case double_read:
                return "Async read already in progress";
            case output_stream_required:
                return "An output stream to be set before async_write can be used";
            case bad_stream:
                return "A stream operation returned ios::bad";
            default:
                return "Unknown";
        }
    }
};

/// Get a reference to a static copy of the iostream transport error category
inline lib::error_category const & get_category() {
    static category instance;
    return instance;
}

/// Get an error code with the given value and the iostream transport category
inline lib::error_code make_error_code(error::value e) {
    return lib::error_code(static_cast<int>(e), get_category());
}

} // namespace error
} // namespace iostream
} // namespace transport
} // namespace websocketpp
_WEBSOCKETPP_ERROR_CODE_ENUM_NS_START_
template<> struct is_error_code_enum<websocketpp::transport::iostream::error::value>
{
    static bool const value = true;
};
_WEBSOCKETPP_ERROR_CODE_ENUM_NS_END_
