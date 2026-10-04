// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

#include <websocketpp/common/asio.hpp>
#include <websocketpp/common/cpp11.hpp>
#include <websocketpp/common/functional.hpp>
#include <websocketpp/common/system_error.hpp>
#include <websocketpp/common/type_traits.hpp>

#include <exception>
#include <limits>
#include <new>
#include <string>

namespace websocketpp {
namespace transport {
/// Transport policy that uses asio
/**
 * This policy uses a single asio io_context to provide transport
 * services to a WebSocket++ endpoint.
 */
namespace asio {

// Class to manage the memory to be used for handler-based custom allocation.
// It contains a single block of memory which may be returned for allocation
// requests. If the memory is in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
class handler_allocator {
public:
    static const size_t size = 1024;
    
    handler_allocator() : m_in_use(false) {}

#ifdef _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_
	handler_allocator(handler_allocator const & cpy) = delete;
	handler_allocator & operator =(handler_allocator const &) = delete;
#endif

    void * allocate(std::size_t memsize) {
        if (!m_in_use && memsize < size) {
            m_in_use = true;
            return static_cast<void*>(&m_storage);
        } else {
            return ::operator new(memsize);
        }
    }

    void deallocate(void * pointer) {
        if (pointer == &m_storage) {
            m_in_use = false;
        } else {
            ::operator delete(pointer);
        }
    }

private:
    // Storage space used for handler-based custom memory allocation.
    lib::aligned_storage<size>::type m_storage;

    // Whether the handler-based custom allocation storage has been used.
    bool m_in_use;
};

// Standard allocator interface used by Asio's associated_allocator trait.
template <typename T>
class handler_allocator_adapter {
public:
    typedef T value_type;

    explicit handler_allocator_adapter(handler_allocator & allocator)
      : m_allocator(&allocator)
    {}

    template <typename U>
    handler_allocator_adapter(handler_allocator_adapter<U> const & other)
      : m_allocator(other.m_allocator)
    {}

    T * allocate(std::size_t count) {
        if (count > (std::numeric_limits<std::size_t>::max)() / sizeof(T)) {
            #ifdef _WEBSOCKETPP_NO_EXCEPTIONS
            std::terminate();
            #else
            throw std::bad_alloc();
            #endif
        }
        return static_cast<T *>(m_allocator->allocate(count * sizeof(T)));
    }

    void deallocate(T * pointer, std::size_t) {
        m_allocator->deallocate(pointer);
    }

    template <typename U>
    bool operator==(handler_allocator_adapter<U> const & other) const {
        return m_allocator == other.m_allocator;
    }

    template <typename U>
    bool operator!=(handler_allocator_adapter<U> const & other) const {
        return !(*this == other);
    }

private:
    template <typename> friend class handler_allocator_adapter;
    handler_allocator * m_allocator;
};

// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. Calls to operator() are forwarded to the
// encapsulated handler.
template <typename Handler>
class custom_alloc_handler {
public:
    typedef handler_allocator_adapter<unsigned char> allocator_type;

    custom_alloc_handler(handler_allocator& a, Handler h)
      : allocator_(a),
        handler_(h)
    {}

    template <typename Arg1>
    void operator()(Arg1 arg1) {
        handler_(arg1);
    }

    template <typename Arg1, typename Arg2>
    void operator()(Arg1 arg1, Arg2 arg2) {
        handler_(arg1, arg2);
    }

    allocator_type get_allocator() const {
        return allocator_type(allocator_);
    }

private:
    handler_allocator & allocator_;
    Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline custom_alloc_handler<Handler> make_custom_alloc_handler(
    handler_allocator & a, Handler h)
{
    return custom_alloc_handler<Handler>(a, h);
}







// Forward declaration of class endpoint so that it can be friended/referenced
// before being included.
template <typename config>
class endpoint;

typedef lib::function<void (lib::asio::error_code const & ec,
    size_t bytes_transferred)> async_read_handler;

typedef lib::function<void (lib::asio::error_code const & ec,
    size_t bytes_transferred)> async_write_handler;

typedef lib::function<void (lib::error_code const & ec)> pre_init_handler;

// handle_timer: dynamic parameters, multiple copies
// handle_proxy_write
// handle_proxy_read
// handle_async_write
// handle_pre_init


/// Asio transport errors
namespace error {
enum value {
    /// Catch-all error for transport policy errors that don't fit in other
    /// categories
    general = 1,

    /// async_read_at_least call requested more bytes than buffer can store
    invalid_num_bytes,

    /// there was an error in the underlying transport library
    pass_through,

    /// The connection to the requested proxy server failed
    proxy_failed,

    /// Invalid Proxy URI
    proxy_invalid,

    /// Invalid host or service
    invalid_host_service
};

/// Asio transport error category
class category : public lib::error_category {
public:
    char const * name() const _WEBSOCKETPP_NOEXCEPT_TOKEN_ {
        return "websocketpp.transport.asio";
    }

    std::string message(int value) const {
        switch(value) {
            case error::general:
                return "Generic asio transport policy error";
            case error::invalid_num_bytes:
                return "async_read_at_least call requested more bytes than buffer can store";
            case error::pass_through:
                return "Underlying Transport Error";
            case error::proxy_failed:
                return "Proxy connection failed";
            case error::proxy_invalid:
                return "Invalid proxy URI";
            case error::invalid_host_service:
                return "Invalid host or service";
            default:
                return "Unknown";
        }
    }
};

/// Get a reference to a static copy of the asio transport error category
inline lib::error_category const & get_category() {
    static category instance;
    return instance;
}

/// Create an error code with the given value and the asio transport category
inline lib::error_code make_error_code(error::value e) {
    return lib::error_code(static_cast<int>(e), get_category());
}

} // namespace error
} // namespace asio
} // namespace transport
} // namespace websocketpp

_WEBSOCKETPP_ERROR_CODE_ENUM_NS_START_
template<> struct is_error_code_enum<websocketpp::transport::asio::error::value>
{
    static bool const value = true;
};
_WEBSOCKETPP_ERROR_CODE_ENUM_NS_END_
