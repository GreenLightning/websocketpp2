// Copyright (c) 2014, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#ifndef WEBSOCKETPP_CONCURRENCY_NONE_HPP
#define WEBSOCKETPP_CONCURRENCY_NONE_HPP

namespace websocketpp {

/// Concurrency handling support
namespace concurrency {

/// Implementation for no-op locking primitives
namespace none_impl {
/// A fake mutex implementation that does nothing
class fake_mutex {
public:
    fake_mutex() {}
    ~fake_mutex() {}
};

/// A fake lock guard implementation that does nothing
class fake_lock_guard {
public:
    explicit fake_lock_guard(fake_mutex) {}
    ~fake_lock_guard() {}
};
} // namespace none_impl

/// Stub concurrency policy that implements the interface using no-ops.
/**
 * This policy documents the concurrency policy interface using no-ops. It can
 * be used as a reference or base for building a new concurrency policy. It can
 * also be used as is to disable all locking for endpoints used in purely single
 * threaded programs.
 */
class none {
public:
    /// The type of a mutex primitive
    /**
     * std::mutex is an example.
     */
    typedef none_impl::fake_mutex mutex_type;

    /// The type of a scoped/RAII lock primitive.
    /**
     * The scoped lock constructor should take a mutex_type as a parameter,
     * acquire that lock, and release it in its destructor. std::lock_guard is
     * an example.
     */
    typedef none_impl::fake_lock_guard scoped_lock_type;
};

} // namespace concurrency
} // namespace websocketpp

#endif // WEBSOCKETPP_CONCURRENCY_ASYNC_HPP
