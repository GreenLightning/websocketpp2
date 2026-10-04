Complete & Tested:
- Server and client roles pass all Autobahn v0.5.9 test suite tests strictly
- Streaming UTF8 validation
- random number generation
- iostream based transport
- C++11 support
- LLVM/Clang support
- GCC support
- 64 bit support
- 32 bit support
- Logging
- Client role
- message_handler
- ping_handler
- pong_handler
- open_handler
- close_handler
- echo_server & echo_server_tls
- External io_service support
- TLS support
- exception/error handling
- Timeouts
- Subprotocol negotiation
- validate_handler
- Outgoing Proxy Support
- socket_init_handler
- tls_init_handler
- tcp_init_handler

Ongoing work
- Build system update (CMake 3 and 4)
- Dependency updates
	- C++ standard
	- Asio
- Performance tuning
- Visual Studio / Windows support
- http_handler

Future feature roadmap
- Extension support
- permessage_compress extension
- Message buffer pool
- flow control
- tutorials & documentation
