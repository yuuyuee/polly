// Copyright 2022 The Oak Authors.

#include "oak/common/system.h"

#include <sched.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>

#include <atomic>

#include "oak/common/fs.h"
#include "oak/common/error_code.h"
#include "oak/common/format.h"
#include "oak/common/throw_delegate.h"
#include "oak/common/debug.h"

namespace oak {

pid_t System::GetThreadId() {
  return syscall(SYS_gettid);
}

pid_t System::GetCachedThreadId() {
  static thread_local pid_t thread_id = GetThreadId();
  return thread_id;
}

namespace {
std::atomic<pid_t> max_logic_thread_id;
pid_t GetLogicThreadIdImpl() {
  pid_t thread_id = max_logic_thread_id.load(std::memory_order_relaxed);
  while (!max_logic_thread_id
      .compare_exchange_strong(
          thread_id, thread_id + 1,
          std::memory_order_acq_rel,
          std::memory_order_relaxed)) {}
  return thread_id;
}
}  // anonymous namespace

pid_t System::GetLogicThreadId() {
  static thread_local pid_t thread_id = GetLogicThreadIdImpl();
  return thread_id;
}

int System::GetCurrentCpu() {
  return sched_getcpu();
}

void System::ThreadYield() {
  sched_yield();
}

void System::SetParentDeathSignal(int signo) {
  int ret = prctl(PR_SET_PDEATHSIG, signo);
  if (ret < 0) {
    ThrowStdSystemError(
        Format("prctl(PR_SET_PDEATHSIG, %d) failed: %s",
               signo, Strerror(errno)));
  }
}

namespace {
#define OAK_MAX_NUMA_NODES (8)
#define OAK_MAX_LOGIC_CORES (128)

// Global CPU layout
struct LogicCoreWrapper {
  bool enable;
  std::atomic<bool> lock;
  struct LogicCore core;
};
LogicCoreWrapper g_logic_core[OAK_MAX_LOGIC_CORES];

void InitLogicCore(LogicCoreWrapper* logic_core, int size) {
  for (int i = 0; i < size; ++i) {
    logic_core[i].enable = false;
    logic_core[i].lock.store(false, std::memory_order_relaxed);
    logic_core[i].core.logic_core_id = i;
    CPU_ZERO(&(logic_core[i].core.mask));
    logic_core[i].core.socket_id = 0;
  }
}

#define OAK_SYS_CPU_DIR  "/sys/devices/system/cpu/"
#define OAK_SYS_NODE_DIR "/sys/devices/system/node/"
#define OAK_SYS_CORE_ID "/topology/core_id"

// Initialize current CPU layout.
int InitCpuLayout() {
  InitLogicCore(g_logic_core, OAK_MAX_LOGIC_CORES);

  for (int i = 0; i < OAK_MAX_LOGIC_CORES; ++i) {
    std::string path = Format(OAK_SYS_CPU_DIR "cpu%d" OAK_SYS_CORE_ID, i);
    if (!IsExists(path))
      continue;
    g_logic_core[i].enable = true;
    CPU_SET(i, &(g_logic_core[i].core.mask));

    for (int j = 0; j < OAK_MAX_NUMA_NODES; ++j) {
      path = Format(OAK_SYS_CPU_DIR "cpu%d/node%d", i, j);
      if (!IsExists(path))
        continue;
      g_logic_core[i].core.socket_id = j;
      break;
    }
  }
  return 0;
}

}  // anonymous namespace

const LogicCore* System::GetNextAvailLogicCore(int core_hint) {
  static const int done = InitCpuLayout();
  (void) done;

  for (int i = 0; i < OAK_MAX_LOGIC_CORES; ++i) {
    LogicCoreWrapper* lcore_wrapper = &(g_logic_core[i]);
    if (!lcore_wrapper->enable)
      continue;

    if (core_hint >= 0 && lcore_wrapper->core.logic_core_id < core_hint)
      continue;

    bool locked = false;
    if (!lcore_wrapper->lock.compare_exchange_strong(
        locked, true,
        std::memory_order_acq_rel, std::memory_order_relaxed)) {
      continue;
    }
    return &lcore_wrapper->core;
  }
  return nullptr;
}

const LogicCore* System::GetCurrentLogicCore() {
  int index = GetCurrentCpu();
  if (index >= OAK_MAX_LOGIC_CORES)
    ThrowStdRuntimeError(Format("Current CPU core %d out of range\n", index));
  return &(g_logic_core[index].core);
}

namespace {
struct RoutineArgs {
  std::string name;
  cpu_set_t favor;
  std::function<void()> fn;

  RoutineArgs(const std::string& name,
              const cpu_set_t& favor,
              std::function<void()> fn)
      : name(name), favor(favor), fn(std::move(fn)) {}
};

void* StartRoutine(void* args) {
  SetSignalAltStack();

  const RoutineArgs rout_args(*reinterpret_cast<RoutineArgs*>(args));
  delete reinterpret_cast<RoutineArgs*>(args);

  if (!rout_args.name.empty())
    oak::System::SetThreadName(pthread_self(), rout_args.name);

  if (CPU_COUNT(&rout_args.favor) > 0) {
    oak::System::SetThreadAffinity(pthread_self(), rout_args.favor);
    oak::System::ThreadYield();
  }

  rout_args.fn();
  return 0;
}

#define PTHREAD_ERROR(err, msg)             \
  if (err != 0) {                           \
    ThrowStdSystemError(                    \
        Format(msg ": %s", Strerror(err))); \
  }

}  // anonymous namespace

pthread_t System::CreateThread(const std::string& name,
                               const cpu_set_t* favor,
                               std::function<void()>&& fn) {
  assert(fn && "Invalid argument");

  pthread_attr_t attr;
  int err = pthread_attr_init(&attr);
  PTHREAD_ERROR(err, "pthread_attr_init() failed");

  err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  PTHREAD_ERROR(err, "pthread_attr_setdetachstate() failed")

  pthread_t pid;
  cpu_set_t local_favor;
  CPU_ZERO(&local_favor);
  if (favor)
    memcpy(&local_favor, &favor, sizeof(*favor));
  RoutineArgs* args = new RoutineArgs(name, local_favor, std::move(fn));
  err = pthread_create(&pid, &attr, StartRoutine,
      const_cast<void*>(static_cast<const void*>(args)));
  PTHREAD_ERROR(err, "pthread_create() failed");

  return pid;
}

void System::SetThreadAffinity(pthread_t id, const cpu_set_t& mask) {
  int err = pthread_setaffinity_np(id, sizeof(cpu_set_t), &mask);
  PTHREAD_ERROR(err, "pthread_setaffinity_np() failed");
}

void System::SetThreadName(pthread_t id, const std::string& name) {
  // The name can be up to 16 bytes long, including the terminating null
  // byte, if the length of the string exceed 16 bytes, the string is
  // silently truncated.
  size_t size = name.size() < 15 ? name.size() : 15;
  char buffer[16];
  memcpy(buffer, name.c_str(), size);
  buffer[size] = '\0';
  int err = pthread_setname_np(id, buffer);
  PTHREAD_ERROR(err, "pthread_setname_np() failed");
}

void System::SaveArgument(int, char* []) {
  // TODO(YUYUE):
}

void System::SetProcessName(const std::string&) {
  // TODO(YUYUE):
}



}  // namespace oak
