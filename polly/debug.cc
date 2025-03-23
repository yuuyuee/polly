// Copyright 2022 The Oak Authors.

#include "oak/common/debug.h"

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <execinfo.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <atomic>
#include <memory>

#include "oak/common/error_code.h"
#include "oak/common/system.h"
#include "oak/common/format.h"
#include "oak/common/fs.h"
#include "oak/common/throw_delegate.h"

namespace oak {
namespace {

bool SetupSignalAltStackImpl() {
  stack_t ss;
  ss.ss_flags = 0;
  // There additional stack space has required.
  ss.ss_size = 8 * SIGSTKSZ;
  ss.ss_sp = mmap(nullptr, ss.ss_size,
                  PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
                  -1, 0);
  if (ss.ss_sp == MAP_FAILED) {
    ThrowStdSystemError(
        Format("map() for alternate signal stack failed: %s",
               Strerror(errno)));
  }
  if (sigaltstack(&ss, nullptr) < 0) {
    ThrowStdSystemError(
        Format("sigaltstack() failed: %s", Strerror(errno)));
  }
  return true;
}

const int kFailureSignal[] = {
  SIGSEGV, SIGILL, SIGFPE, SIGBUS, SIGABRT
};

const char* StrSignal(int signo) {
  return signo == SIGSEGV ? "SIGSEGV"
       : signo == SIGILL ? "SIGILL"
       : signo == SIGFPE ? "SIGFPE"
       : signo == SIGBUS ? "SIGBUS"
       : signo == SIGABRT ? "SIGABRT"
       : "UNKNOWN";
}

// Failure message writer.
std::unique_ptr<File> writer;

void ResetSignalHandlerAndRaise(int signo) {
  signal(signo, SIG_DFL);
  raise(signo);
}

// Current thread ID.
std::atomic<pid_t> failed_tid(0);

void FailureSignalHandler(int signo, siginfo_t* info, void*) {
  const pid_t this_tid = oak::System::GetThreadId();
  pid_t prev_tid = 0;
  if (!failed_tid.compare_exchange_strong(
        prev_tid, this_tid,
        std::memory_order_acq_rel, std::memory_order_relaxed)) {
    if (prev_tid != this_tid)
      return;
  }

  if (!writer) {
    ResetSignalHandlerAndRaise(signo);
    return;
  }

  // Write title
  char title[128];
  size_t len = format(title, sizeof(title),
                      "*** %s received at %ld, pid = %d, uid = %d ***\n",
                      StrSignal(signo),
                      static_cast<long int>(time(0)),
                      info->si_pid, info->si_uid);
  writer->Write(title, len);

  // Write stack frames
  void* frame[32];
  int num_frame = backtrace(frame, 32);
  // TODO(YUYUE): demangling for symbols
  backtrace_symbols_fd(frame, num_frame, writer->fd());

  // Write ends
  static const char ends[] = "*** ends ***\n";
  writer->Write(ends, sizeof(ends) - 1);

  ResetSignalHandlerAndRaise(signo);
}

void RegisterFailureSignalHandlerOnce(int signo) {
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  sigemptyset(&act.sa_mask);
  act.sa_flags |= SA_SIGINFO | SA_NODEFER | SA_ONSTACK;
  act.sa_sigaction = FailureSignalHandler;
  if (sigaction(signo, &act, nullptr) < 0) {
    ThrowStdSystemError(
        Format("sigaction() failed: %s", Strerror(errno)));
  }
}

}  // anonymous namespace

bool SignalAltStackEnabled() {
  stack_t ss;
  if (sigaltstack(nullptr, &ss) < 0) {
    ThrowStdSystemError(
        Format("sigaltstack() failed: %s", Strerror(errno)));
  }
  return !(ss.ss_flags & SS_DISABLE);
}

bool SetSignalAltStack() {
  static thread_local bool enable = SetupSignalAltStackImpl();
  return enable;
}

void RegisterFailureMessageHandler(int fd) {
  writer.reset(new File(fd));
}

void RegisterFailureMessageHandler(const char* fname) {
  writer.reset(new File(fname, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC));
}

void RegisterFailureMessageHandler(const std::string& fname) {
  RegisterFailureMessageHandler(fname.c_str());
}

void RegisterFailureSignalHandler() {
  for (auto signo : kFailureSignal)
    RegisterFailureSignalHandlerOnce(signo);
}

}  // namespace oak
