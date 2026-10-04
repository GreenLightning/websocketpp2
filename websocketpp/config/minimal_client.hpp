// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_CONFIG_MINIMAL_CLIENT_HPP
#define WEBSOCKETPP_CONFIG_MINIMAL_CLIENT_HPP

#include <websocketpp/config/minimal_server.hpp>

namespace websocketpp {
namespace config {

/// Client config with minimal dependencies
/**
 * This config strips out as many dependencies as possible. It is suitable for
 * use as a base class for custom configs that want to implement or choose their
 * own policies for components that even the core config includes.
 *
 * NOTE: this config stubs out enough that it cannot be used directly. You must
 * supply at least a transport policy and a cryptographically secure random 
 * number generation policy for a config based on `minimal_client` to do 
 * anything useful.
 *
 * Present dependency list for minimal_server config:
 *
 * C++98 STL:
 * <algorithm>
 * <map>
 * <sstream>
 * <string>
 * <vector>
 *
 * C++11 STL or Boost
 * <memory>
 * <functional>
 * <system_error>
 *
 * Operating System:
 * <stdint.h>
 * <netinet/in.h> or <winsock2.h> (for ntohl.. could potentially bundle this)
 *
 * @since 0.4.0-dev
 */
typedef minimal_server minimal_client;

} // namespace config
} // namespace websocketpp

#endif // WEBSOCKETPP_CONFIG_MINIMAL_CLIENT_HPP
