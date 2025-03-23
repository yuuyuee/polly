// Copyright 2022 The Oak Authors.

#ifndef POLLY_BOOST_ASIO_H_
#define POLLY_BOOST_ASIO_H_

// Disable deprecated interfaces and functionality.
#define BOOST_ASIO_NO_DEPRECATED 1

// Use Networking TS-style polymorphic wrapper.
#define BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT 1

// Disable support for the DynamicBuffer_v1 type requirements.
#define BOOST_ASIO_NO_DYNAMIC_BUFFER_V1 1

#define BOOST_ASIO_NO_TYPEID 1

#include "boost/asio.h"

#endif  // POLLY_BOOST_ASIO_H_
