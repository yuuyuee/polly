// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#include "rctems/common/asio.h"

#include <mutex>  // NOLINT
#include <utility>
#include "rctems/common/logging.h"

namespace rctems {

Timer::Timer(asio::io_context* context, std::function<void()>&& func)
        : handler_(*context), func_(std::move(func)) {}

void Timer::Start(Duration expiry_time, bool repeat) {
    handler_.expires_after(expiry_time);
    auto self = shared_from_this();
    handler_.async_wait([self, expiry_time, repeat] (const auto& ec) {
        if (ec) {
            if (ec == asio::error::operation_aborted)
                return;
            RCTEMS_FATAL() << "Timer::Start(): " << ec.message();
            std::abort();
        }

        if (self->func_)
            self->func_();

        if (repeat)
            self->Start(expiry_time, repeat);
    });
}

void Timer::Stop() {
    handler_.cancel();
}

}  // namespace rctems
