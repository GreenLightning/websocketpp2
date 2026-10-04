WebSocket++ 2.0.0-dev
====================

_Fork and continuation of the original (now mostly inactive)
[websocketpp](https://github.com/zaphoyd/websocketpp) project. The goal of
this fork is to modernize and evolve the library while providing a
straightforward upgrade path for existing users. We are AI-positive._

WebSocket++ is a header only C++ library that implements RFC6455 The WebSocket
Protocol. It allows integrating WebSocket client and server functionality into
C++ programs. It uses interchangeable network transport modules including one
based on raw char buffers, one based on C++ iostreams, and one based on Asio 
(either via Boost or standalone). End users can write additional transport
policies to support other networking or event libraries as needed.

Major Features
==============

* Full support for RFC6455 (WebSocket version 13 only)
* Message/event based interface
* Supports secure WebSockets (TLS), IPv6, and explicit proxies.
* Requires C++11 or later; supports standalone Asio and Boost.Asio
* Interchangeable network transport modules (raw, iostream, Asio, or custom)
* Portable/cross platform (Posix/Windows, 32/64bit, Intel/ARM/PPC)
* Thread-safe

Requirements and upgrading
=========================

WebSocket++ requires a C++11 compiler and standard library or later. C++98/03
and Boost replacements for standard-library types are no longer supported.
Standard-library `<thread>`, `<mutex>`, and `<condition_variable>` support is
required on all platforms, including MinGW. The `mingw-std-threads` fallback
and its `_WEBSOCKETPP_MINGW_THREAD_` switch have been removed.
The `websocketpp::lib` aliases remain available for source compatibility:
`shared_ptr`, `weak_ptr`, `unique_ptr`, `enable_shared_from_this`, `function`,
`bind`, threading types, `chrono`, random-number types, regular expressions,
and type traits now always use their standard-library equivalents. For example,
`websocketpp::lib::shared_ptr<T>` is always `std::shared_ptr<T>`.

Error types follow the selected Asio backend, independently of the C++ standard:

| Backend | Selection | Minimum version | `websocketpp::lib::error_code` |
| --- | --- | --- | --- |
| Standalone Asio | Define `ASIO_STANDALONE` | Asio 1.8.0 | `std::error_code` |
| Boost.Asio | Leave `ASIO_STANDALONE` undefined | Boost 1.49.0 | `boost::system::error_code` |

The `error_category`, `error_condition`, `system_error`, and `errc` aliases use
the same backend. Define `ASIO_STANDALONE` consistently across all translation
units, before including any Asio or WebSocket++ headers. With this define, the
iostream/raw transports require neither Boost nor Asio headers.

When upgrading:

* Compile with C++11 or later (for example, `-std=c++11`). CMake defaults to
  C++11 and preserves newer standards; the installed `websocketpp::websocketpp`
  target propagates the C++11 requirement. The optional `ENABLE_CPP11` and
  `WSPP_ENABLE_CPP11` settings have been removed.
* Replace Boost pointers, callbacks, threads, and other standard-library
  substitutes passed to WebSocket++ with `std::` types or `websocketpp::lib`
  aliases. The `_WEBSOCKETPP_NO_CPP11_*` switches no longer select Boost types.
* Use `websocketpp::lib::error_code` in handlers and output parameters to match
  the selected backend. C++11 builds using Boost.Asio now use Boost.System error
  types; the former C++11 error-type selection switches have no effect.

License
=======

3-Clause BSD (See COPYING for more details)

Author
======

Peter Thorson - websocketpp@zaphoyd.com
