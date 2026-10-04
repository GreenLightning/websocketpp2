// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/platforms.hpp>
#include <websocketpp/common/stdint.hpp>
#include <websocketpp/common/system_error.hpp>

#include <websocketpp/http/constants.hpp>
#include <websocketpp/extensions/extension.hpp>

#include <map>
#include <string>
#include <utility>

namespace websocketpp {
namespace extensions {
namespace permessage_deflate {

/// Stub class for use when disabling permessage_deflate extension
/**
 * This class is a stub that implements the permessage_deflate interface
 * with minimal dependencies. It is used to disable permessage_deflate
 * functionality at compile time without loading any unnecessary code.
 */
template <typename config>
class disabled {
    typedef std::pair<lib::error_code,std::string> err_str_pair;

public:
    /// Negotiate extension
    /**
     * The disabled extension always fails the negotiation with a disabled
     * error.
     *
     * @param offer Attribute from client's offer
     * @return Status code and value to return to remote endpoint
     */
    err_str_pair negotiate(http::attribute_list const &) {
        return make_pair(make_error_code(error::disabled),std::string());
    }

    /// Initialize state
    /**
     * For the disabled extension state initialization is a no-op.
     *
     * @param is_server True to initialize as a server, false for a client.
     * @return A code representing the error that occurred, if any
     */
    lib::error_code init(bool) {
        return lib::error_code();
    }

    /// Returns true if the extension is capable of providing
    /// permessage_deflate functionality
    bool is_implemented() const {
        return false;
    }

    /// Returns true if permessage_deflate functionality is active for this
    /// connection
    bool is_enabled() const {
        return false;
    }

    /// Generate extension offer
    /**
     * Creates an offer string to include in the Sec-WebSocket-Extensions
     * header of outgoing client requests.
     *
     * @return A WebSocket extension offer string for this extension
     */
    std::string generate_offer() const {
        return "";
    }

    /// Set maximum decompressed message size (no-op)
    /**
     * Provided for API parity with the enabled extension. The disabled
     * extension never decompresses, so there is no limit to enforce and
     * the value is silently ignored.
     *
     * @since 0.8.3
     */
    void set_max_message_size(size_t) {}

    /// Compress bytes
    /**
     * @param [in] in String to compress
     * @param [out] out String to append compressed bytes to
     * @return Error or status code
     */
    lib::error_code compress(std::string const &, std::string &) {
        return make_error_code(error::disabled);
    }

    /// Decompress bytes
    /**
     * @param buf Byte buffer to decompress
     * @param len Length of buf
     * @param out String to append decompressed bytes to
     * @return Error or status code
     */
    lib::error_code decompress(uint8_t const *, size_t, std::string &) {
        return make_error_code(error::disabled);
    }

    static bool is_message_too_big(lib::error_code const &) {
        return false;
    }
};

} // namespace permessage_deflate
} // namespace extensions
} // namespace websocketpp
