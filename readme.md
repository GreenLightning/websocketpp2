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
| Standalone Asio | Define `ASIO_STANDALONE` | Asio 1.12.0 | `std::error_code` |
| Boost.Asio | Leave `ASIO_STANDALONE` undefined | Boost 1.66.0 | `boost::system::error_code` |

The `error_category`, `error_condition`, `system_error`, and `errc` aliases use
the same backend. Define `ASIO_STANDALONE` consistently across all translation
units, before including any Asio or WebSocket++ headers. With this define, the
iostream/raw transports require neither Boost nor Asio headers.

The Asio transport uses `io_context`, executor work guards, and executor-bound
strands. Use `get_io_context()` to access the event loop and `restart()` before
running it again after it has stopped. The former `get_io_service()`,
`io_service_ptr`, and `reset()` names remain as compatibility wrappers. Custom
socket policies must use `asio::strand<asio::io_context::executor_type>` for
their strand pointers.

When upgrading:

* Compile with C++11 or later (for example, `-std=c++11`). The
  `websocketpp::websocketpp` CMake target propagates this minimum requirement
  for both source-tree and installed consumers, while preserving newer standards.
  The optional `ENABLE_CPP11` and `WSPP_ENABLE_CPP11` settings have been removed.
* Replace `_WEBSOCKETPP_NOEXCEPT_TOKEN_`, `_WEBSOCKETPP_CONSTEXPR_TOKEN_`, and
  `_WEBSOCKETPP_NULLPTR_TOKEN_` with `noexcept`, `constexpr`, and `nullptr`.
  The C++11 feature macros have been removed; move semantics, deleted functions,
  initializer lists, and `std::put_time` are now used unconditionally.
  Replace direct includes of `websocketpp/common/cpp11.hpp` with
  `websocketpp/common/platforms.hpp`, which now checks compiler requirements.
* Replace Boost pointers, callbacks, threads, and other standard-library
  substitutes passed to WebSocket++ with `std::` types or `websocketpp::lib`
  aliases. The `_WEBSOCKETPP_NO_CPP11_*` switches no longer select Boost types.
* Use `websocketpp::lib::error_code` in handlers and output parameters to match
  the selected backend. C++11 builds using Boost.Asio now use Boost.System error
  types; the former C++11 error-type selection switches have no effect.

Building with CMake
===================

CMake 3.18 or later is required. To build the examples and tests:

```sh
cmake -S . -B build -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
(cd build && ctest --output-on-failure)
```

Choose the build type explicitly for single-configuration generators; no default
is imposed. Boost is required for examples and tests. Test builds also require
OpenSSL and zlib so the full suite is always built. For builds with only examples,
OpenSSL and zlib enable their corresponding targets when available. Use `-DBOOST_STATIC=ON`
for static Boost libraries. Installing just the header-only library requires
none of these dependencies.

Compression and TLS tests have separate executables. Run their groups with
`ctest --test-dir build -L compression` or `ctest --test-dir build -L tls`.

Link to `websocketpp::websocketpp` after either `add_subdirectory()` or
`find_package(websocketpp CONFIG REQUIRED)`. The target supplies the headers and
C++11 requirement; applications select and link their own transport dependencies.

Browser example
===============

Open [examples/browser_client/index.html](examples/browser_client/index.html)
directly in a modern browser. It uses the native WebSocket API and needs no
JavaScript dependencies or build step. Build and run an echo server from the
repository root:

```sh
cmake -S . -B build -DBUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target echo_server
./build/bin/echo_serverd
```

Connect to `ws://localhost:9002`, then send a text message to see its echo.
Debug executables have a `d` suffix; Release executables do not. For a
multi-configuration generator, also pass `--config Debug` when building and
run the executable from `build/bin/Debug/`.

Alternatively, build and run `simple_broadcast_server` or `broadcast_server`
and connect from two browser tabs to exchange messages. Run one server at a
time on the default port. To receive telemetry, build `telemetry_server` and run
`./build/bin/telemetry_serverd examples/telemetry_server/ 9002`; the trailing
slash on the document root is required by that example. The browser client
will display the counter messages once connected.

For a TLS server, enter its `wss://` URL and use a certificate trusted by the
browser. If you serve the browser client over HTTPS, use `wss://` to avoid
mixed-content restrictions.

Author
======

Peter Thorson - websocketpp@zaphoyd.com
