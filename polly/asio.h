// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#ifndef RCTEMS_COMMON_ASIO_H_
#define RCTEMS_COMMON_ASIO_H_

#include <chrono>  // NOLINT
#include <memory>
#include <functional>

#include "rctems/common/stubs.h"

#if RCTEMS_HAS_INCLUDE("asio.hpp")
// Disable deprecated interfaces and functionality.
#   define ASIO_NO_DEPRECATED 1

// Use Networking TS-style polymorphic wrapper.
#   define ASIO_USE_TS_EXECUTOR_AS_DEFAULT 1

// Disable support for the DynamicBuffer_v1 type requirements.
#   define ASIO_NO_DYNAMIC_BUFFER_V1 1

#   define ASIO_STANDALONE 1
#   define ASIO_NO_TYPEID 1
#   define ASIO_DISABLE_VISIBILITY 1

#include "asio.hpp"  // NOLINT
# else
// Disable deprecated interfaces and functionality.
#   define BOOST_ASIO_NO_DEPRECATED 1

// Use Networking TS-style polymorphic wrapper.
#   define BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT 1

// Disable support for the DynamicBuffer_v1 type requirements.
#   define BOOST_ASIO_NO_DYNAMIC_BUFFER_V1 1

#   define BOOST_ASIO_STANDALONE 1
#   define BOOST_ASIO_NO_TYPEID 1
#   define BOOST_ASIO_DISABLE_VISIBILITY 1

#include "boost/asio.hpp"

namespace asio = boost::asio;
#endif

namespace rctems {

class Timer: public std::enable_shared_from_this<Timer> {
 public:
    using Duration = asio::steady_timer::duration;

    explicit Timer(asio::io_context* context, std::function<void()>&& func);

    void Start(const Duration expiry_time, bool repeat);
    void Stop();

 private:
    asio::steady_timer handler_;
    std::function<void()> func_;

    RCTEMS_DISALLOW_COPY_AND_ASSIGN(Timer);
};
}  // namespace rctems

#endif  // RCTEMS_COMMON_ASIO_H_
