// Copyright (c) 2015, Peter Thorson
// SPDX-License-Identifier: BSD-3-Clause
// See LICENSE.txt for the full license text.

#pragma once

// NOTE: This file must be included before common/asio.hpp

#ifdef ASIO_STANDALONE
    #include <asio/ssl.hpp>
#else
    #include <boost/asio/ssl.hpp>
#endif
