// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

// The initial version of this logging policy was contributed to the WebSocket++
// project by Tom Hughes.

#pragma once

#include <syslog.h>

#include <websocketpp/logger/basic.hpp>

#include <websocketpp/common/cpp11.hpp>
#include <websocketpp/logger/levels.hpp>

namespace websocketpp {
namespace log {

/// Basic logger that outputs to syslog
template <typename concurrency, typename names>
class syslog : public basic<concurrency, names> {
public:
    typedef basic<concurrency, names> base;

    /// Construct the logger
    /**
     * @param hint A channel type specific hint for how to construct the logger
     */
    syslog<concurrency,names>(channel_type_hint::value hint =
        channel_type_hint::access)
      : basic<concurrency,names>(hint), m_channel_type_hint(hint) {}

    /// Construct the logger
    /**
     * @param channels A set of channels to statically enable
     * @param hint A channel type specific hint for how to construct the logger
     */
    syslog<concurrency,names>(level channels, channel_type_hint::value hint =
        channel_type_hint::access)
      : basic<concurrency,names>(channels, hint), m_channel_type_hint(hint) {}

    /// Write a string message to the given channel
    /**
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level channel, std::string const & msg) {
        write(channel, msg.c_str());
    }

    /// Write a cstring message to the given channel
    /**
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level channel, char const * msg) {
        scoped_lock_type lock(base::m_lock);
        if (!this->dynamic_test(channel)) { return; }
        ::syslog(syslog_priority(channel), "[%s] %s",
            names::channel_name(channel), msg);
    }
private:
    typedef typename base::scoped_lock_type scoped_lock_type;

    /// The default level is used for all access logs and any error logs that
    /// don't trivially map to one of the standard syslog levels.
    static int const default_level = LOG_INFO;

    /// retrieve the syslog priority code given a WebSocket++ channel
    /**
     * @param channel The level to look up
     * @return The syslog level associated with `channel`
     */
    int syslog_priority(level channel) const {
        if (m_channel_type_hint == channel_type_hint::access) {
            return syslog_priority_access(channel);
        } else {
            return syslog_priority_error(channel);
        }
    }

    /// retrieve the syslog priority code given a WebSocket++ error channel
    /**
     * @param channel The level to look up
     * @return The syslog level associated with `channel`
     */
    int syslog_priority_error(level channel) const {
        switch (channel) {
            case elevel::devel:
                return LOG_DEBUG;
            case elevel::library:
                return LOG_DEBUG;
            case elevel::info:
                return LOG_INFO;
            case elevel::warn:
                return LOG_WARNING;
            case elevel::rerror:
                return LOG_ERR;
            case elevel::fatal:
                return LOG_CRIT;
            default:
                return default_level;
        }
    }

    /// retrieve the syslog priority code given a WebSocket++ access channel
    /**
     * @param channel The level to look up
     * @return The syslog level associated with `channel`
     */
    _WEBSOCKETPP_CONSTEXPR_TOKEN_ int syslog_priority_access(level) const {
        return default_level;
    }

    channel_type_hint::value m_channel_type_hint;
};

} // log
} // websocketpp
