// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#ifndef POLLY_COMMON_DEBUG_H_
#define POLLY_COMMON_DEBUG_H_

#include <string>

namespace polly {
// Signale alternate stack.

// Checks whether or not the signal alternate stack has been enabled.
bool SignalAltStackEnabled();

// Setup signal alternate stack, there is safety for multiple calls.
bool SetSignalAltStack();

// Register failure message handler.
void RegisterFailureMessageHandler(int fd);
void RegisterFailureMessageHandler(const char* fname);
void RegisterFailureMessageHandler(const std::string& fname);

// Register a failure signal message writer for common failure signal
// such as below:
//    SIGSEGV, SIGILL, SIGFPE, SIGBUS, SIGABRT
// Wites failure message useful for debugging, if unspecified writer,
// the failure message will be emitted to the standard error output.
void RegisterFailureSignalHandler();
}  // namespace polly

#endif  // POLLY_COMMON_DEBUG_H_
