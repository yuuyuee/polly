// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_DEBUG_H_
#define OAK_COMMON_DEBUG_H_

#include <string>

namespace oak {
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
}  // namespace oak

#endif  // OAK_COMMON_DEBUG_H_
