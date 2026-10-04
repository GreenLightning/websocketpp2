// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

namespace websocketpp {
/// Random number generation policies
namespace random {
/// Stub RNG policy that always returns 0
namespace none {

/// Thread safe stub "random" integer generator.
/**
 * This template class provides a random integer stub. The interface mimics the
 * WebSocket++ RNG generator classes but the generater function always returns
 * zero. This can be used to stub out the RNG for unit and performance testing.
 *
 * Call operator() to generate the next number
 */
template <typename int_type>
class int_generator {
    public:
        int_generator() {}

        /// advances the engine's state and returns the generated value
        int_type operator()() {
            return 0;
        }
};

} // namespace none
} // namespace random
} // namespace websocketpp
