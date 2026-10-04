// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_RANDOM_RANDOM_DEVICE_HPP
#define WEBSOCKETPP_RANDOM_RANDOM_DEVICE_HPP

#include <websocketpp/common/random.hpp>

namespace websocketpp {
namespace random {
/// RNG policy based on std::random_device or boost::random_device
namespace random_device {

/// Thread safe non-deterministic random integer generator.
/**
 * This template class provides thread safe non-deterministic random integer
 * generation. Numbers are produced in a uniformly distributed range from the
 * smallest to largest value that int_type can store.
 *
 * Thread-safety is provided via locking based on the concurrency template
 * parameter.
 *
 * Non-deterministic RNG is provided via websocketpp::lib which uses either
 * C++11 or Boost 1.47+'s random_device class.
 *
 * Call operator() to generate the next number
 */
template <typename int_type, typename concurrency>
class int_generator {
    public:
        typedef typename concurrency::scoped_lock_type scoped_lock_type;
        typedef typename concurrency::mutex_type mutex_type;

        /// constructor
        //mac TODO: figure out if signed types present a range problem
        int_generator() {}

        /// advances the engine's state and returns the generated value
        int_type operator()() {
            scoped_lock_type guard(m_lock);
            return m_dis(m_rng);
        }
    private:


        lib::random_device m_rng;
        lib::uniform_int_distribution<int_type> m_dis;

        mutex_type m_lock;
};

} // namespace random_device
} // namespace random
} // namespace websocketpp

#endif //WEBSOCKETPP_RANDOM_RANDOM_DEVICE_HPP
