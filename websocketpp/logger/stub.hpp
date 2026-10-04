// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/logger/levels.hpp>

#include <websocketpp/common/cpp11.hpp>

#include <string>

namespace websocketpp {
namespace log {

/// Stub logger that ignores all input
class stub {
public:
    /// Construct the logger
    /**
     * @param hint A channel type specific hint for how to construct the logger
     */
    explicit stub(channel_type_hint::value) {}

    /// Construct the logger
    /**
     * @param default_channels A set of channels to statically enable
     * @param hint A channel type specific hint for how to construct the logger
     */
    stub(level, channel_type_hint::value) {}
    _WEBSOCKETPP_CONSTEXPR_TOKEN_ stub() {}

    /// Dynamically enable the given list of channels
    /**
     * All operations on the stub logger are no-ops and all arguments are
     * ignored
     *
     * @param channels The package of channels to enable
     */
    void set_channels(level) {}

    /// Dynamically disable the given list of channels
    /**
     * All operations on the stub logger are no-ops and all arguments are
     * ignored
     *
     * @param channels The package of channels to disable
     */
    void clear_channels(level) {}

    /// Write a string message to the given channel
    /**
     * Writing on the stub logger is a no-op and all arguments are ignored
     *
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level, std::string const &) {}

    /// Write a cstring message to the given channel
    /**
     * Writing on the stub logger is a no-op and all arguments are ignored
     *
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level, char const *) {}

    /// Test whether a channel is statically enabled
    /**
     * The stub logger has no channels so all arguments are ignored and
     * `static_test` always returns false.
     *
     * @param channel The package of channels to test
     */
    _WEBSOCKETPP_CONSTEXPR_TOKEN_ bool static_test(level) const {
        return false;
    }

    /// Test whether a channel is dynamically enabled
    /**
     * The stub logger has no channels so all arguments are ignored and
     * `dynamic_test` always returns false.
     *
     * @param channel The package of channels to test
     */
    bool dynamic_test(level) {
        return false;
    }
};

} // log
} // websocketpp
