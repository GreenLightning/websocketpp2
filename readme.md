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

* Full support for RFC6455
* Partial support for Hixie 76 / Hybi 00, 07-17 draft specs (server only)
* Message/event based interface
* Supports secure WebSockets (TLS), IPv6, and explicit proxies.
* Flexible dependency management (C++11 Standard Library or Boost)
* Interchangeable network transport modules (raw, iostream, Asio, or custom)
* Portable/cross platform (Posix/Windows, 32/64bit, Intel/ARM/PPC)
* Thread-safe

License
=======

3-Clause BSD (See COPYING for more details)

Author
======

Peter Thorson - websocketpp@zaphoyd.com
