// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#include "polly/asio.h"

#include <mutex>  // NOLINT
#include <utility>
#include "polly/logging.h"

namespace polly {

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

}  // namespace polly
