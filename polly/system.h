// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_SYSTEM_H_
#define OAK_COMMON_SYSTEM_H_

#include <unistd.h>
#include <pthread.h>

#include <string>
#include <functional>
#include <utility>

namespace oak {

struct LogicCore {
  int logic_core_id;
  cpu_set_t mask;
  int socket_id;
};

struct System {
  // Returns the native thread identification.
  static pid_t GetThreadId();

  // Like as GetThreadId() but not safe to call in an asynchronous
  // signal handling context nor right after a call to fork().
  static pid_t GetCachedThreadId();

  // Returns the logic thread identification within process. This
  // function is not safe to call in an asynchronous signal handling
  // content nor right after a call to fork().
  static pid_t GetLogicThreadId();

  // Returns CPU on which the calling thread is running.
  static int GetCurrentCpu();

  // Yield the calling thread to relinquish the CPU.
  static void ThreadYield();

  // Setup the parent process death signal of the calling process.
  static void SetParentDeathSignal(int signo);

  // Get the point reference next the available logic core.
  static const LogicCore* GetNextAvailLogicCore(int core_hint = -1);

  // Get the point reference current logic core.
  static const LogicCore* GetCurrentLogicCore();

  // Create a new thread named @name, starts execution by invoking
  // @routine running in core @favor. if @name is empty meaning that
  // do not setting the name for the new thread. if @favor is empty
  // meaning that do not setting the CPU affinity for the new thread.
  static pthread_t CreateThread(const std::string& name,
                                const cpu_set_t* favor,
                                std::function<void()>&& fn);

  // Setup the affinity of the thread.
  static void SetThreadAffinity(pthread_t id, const cpu_set_t& mask);

  // Setup the name of the thread.
  static void SetThreadName(pthread_t id, const std::string& name);

  // Setup the name of the process. which maybe should call SaveArgument
  // to saves the argument and environment.
  static void SaveArgument(int argc, char* argv[]);
  static void SetProcessName(const std::string& proc_name);
};

}  // namespace oak

#endif  // OAK_COMMON_SYSTEM_H_
