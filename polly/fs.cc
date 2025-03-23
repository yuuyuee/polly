// Copyright 2022 The Oak Authors.

#include "oak/common/fs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include "oak/common/error_code.h"
#include "oak/common/format.h"
#include "oak/common/throw_delegate.h"

namespace oak {
#define THROW_SYSTEM_ERROR(...) \
    oak::ThrowStdSystemError(oak::Format(__VA_ARGS__))

std::string DirectoryName(const std::string& path) {
  assert(!path.empty() && "Invalid path");
  char real_path[PATH_MAX];
  int count = 0, len = 0;
  for (const auto& v : path) {
    count = v == '/' ? count + 1 : 0;
    if (count > 1)
      continue;
    real_path[len++] = v;
  }
  if (real_path[len - 1] == '/')
    --len;
  real_path[len] = '\0';
  auto pos = strrchr(real_path, '/');
  return pos ? std::string(real_path, pos - real_path) : ".";
}

std::string Basename(const std::string& path) {
  auto pos = path.rfind('/');
  return pos != std::string::npos ? path.substr(pos) : path;
}

std::string GetCurrentDirectory() {
  char path[PATH_MAX];
  const char* p = getcwd(path, PATH_MAX);
  if (!p) {
    THROW_SYSTEM_ERROR("GetCurrentDirectory() failed: %s",
                       Strerror(errno));
  }
  return std::string(path);
}

std::string GetRealPath(const std::string& path) {
  assert(!path.empty() && "Invalid path");
  char real_path[PATH_MAX];
  const char* p = realpath(path.c_str(), real_path);
  if (!p) {
    THROW_SYSTEM_ERROR("GetRealPath(%s) failed: %s",
                       path.c_str(), Strerror(errno));
  }
  return std::string(real_path);
}

void CreateDirectory(const std::string& directory) {
  assert(!directory.empty() && "Invalid directory name");
  static unsigned int mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
  int ret = mkdir(directory.c_str(), mode);
  if (ret < 0 && errno != EEXIST) {
    THROW_SYSTEM_ERROR("CreateDirectory(%s) failed: %s",
                       directory.c_str(), Strerror(errno));
  }
}

void CreateDirectoryRecursively(const std::string& directory) {
  assert(!directory.empty() && "Invalid directory name");
  static unsigned int mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
  int ret = mkdir(directory.c_str(), mode);
  if (ret < 0) {
    if (errno == EEXIST) {
      // XXX: directory already exists but not necessarily as a directory.
      return;
    } else if (errno == ENOENT) {
      std::string path = DirectoryName(directory);
      CreateDirectoryRecursively(path);
      ret = mkdir(directory.c_str(), mode);
      if (ret < 0)
        THROW_SYSTEM_ERROR("CreateDirectoryRecursively(%s) failed: %s",
                           directory.c_str(), Strerror(errno));
    } else {
      THROW_SYSTEM_ERROR("CreateDirectoryRecursively(%s) failed: %s",
                         directory.c_str(), Strerror(errno));
    }
  }
}

void ChangeWorkDirectory(const std::string& directory) {;
  int ret = chdir(directory.c_str());
  if (ret < 0) {
    THROW_SYSTEM_ERROR("ChangeWorkDirectory(%s) failed: %s",
                       directory.c_str(), Strerror(errno));
  }
}

bool IsExists(const std::string& path) {
  return access(path.c_str(), F_OK) == 0;
}

// File

const int File::kMode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

File File::MakeReadOnlyFile(const char* name) {
  return File(name, O_RDONLY | O_CLOEXEC, 0);
}

File File::MakeWritableFile(const char* name) {
  return File(name, O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC, kMode);
}

File File::MakeAppendableFile(const char* name) {
  return File(name, O_WRONLY | O_CREAT | O_CLOEXEC | O_APPEND, kMode);
}

File File::MakeRandomAccessFile(const char* name) {
  return File(name, O_RDWR | O_CREAT | O_CLOEXEC, kMode);
}

File::File(int fd, bool owner) noexcept : fd_(fd), owner_(owner) {
  if (fd_ < -1) assert("fd must be -1 or non-negative value");
  if (fd_ == -1 && owner) assert("can not owned -1");
}

File::File(const char* name, int flags, int mode)
    : fd_(open(name, flags, mode)), owner_(false) {
  if (fd_ < 0) {
    THROW_SYSTEM_ERROR("open(%s, %04o, %04o) failed: %s",
                       name, flags, mode, Strerror(errno));
  }
  owner_ = true;
}

File::File(const std::string& name, int flags, int mode)
    : File(name.c_str(), flags, mode) {}

File::~File() {
  Close();
}

File::File(File&& other) noexcept
    : fd_(other.fd_), owner_(other.owner_) {
  other.Release();
}

File& File::operator=(File&& other) {
  if (this != &other) {
    this->Close();
    fd_ = other.fd_;
    owner_ = other.owner_;
    other.Release();
  }
  return *this;
}

void File::Close() {
  if (fd_ != -1 && owner_) {
    // XXX: all of error has been ignored
    close(fd_);
  }
  Release();
}

size_t File::Read(void* buffer, size_t size) {
  assert(size > 0 && "Invalid size");
  ssize_t ret = 0;
  do {
    ret = read(fd_, buffer, size);
  } while (ret < 0 && errno == EINTR);
  if (ret < 0) {
    THROW_SYSTEM_ERROR("read(%d, %p, %ld) failed: %s",
                       fd_, buffer, size, Strerror(errno));
  }
  return ret;
}

size_t File::Write(const void* buffer, size_t size) {
  assert(size > 0 && "Invalid size");
  ssize_t ret = 0;
  do {
    ret = write(fd_, buffer, size);
  } while (ret < 0 && errno == EINTR);
  if (ret < 0) {
    THROW_SYSTEM_ERROR("write(%d, %p, %ld) failed: %s",
                       fd_, buffer, size, Strerror(errno));
  }
  return ret;
}

off_t File::Seek(off_t offset, int whence) {
  off_t ret = lseek(fd_, offset, whence);
  if (ret < 0) {
    THROW_SYSTEM_ERROR("lseek(%d, %ld, %d) failed: %s",
                       fd_, offset, whence, Strerror(errno));
  }
  return ret;
}

void File::Sync() {
  int ret = fsync(fd_);
  if (ret < 0) {
    THROW_SYSTEM_ERROR("fsync(%d) failed: %s", fd_, Strerror(errno));
  }
}

bool File::TryLock() {
  int ret = flock(fd_, LOCK_EX | LOCK_NB);
  if (ret < 0 && errno != EWOULDBLOCK) {
    THROW_SYSTEM_ERROR("flock(%d, LOCK_EX | LOCK_NB) failed: %s",
                       fd_, Strerror(errno));
  }
  return ret == 0;
}

void File::Unlock() {
  int ret = flock(fd_, LOCK_UN);
  if (ret < 0) {
    THROW_SYSTEM_ERROR("flock(%d, LOCK_UN) failed: %s",
                       fd_, Strerror(errno));
  }
}

int File::Release() noexcept {
  int fd = fd_;
  fd_ = -1;
  owner_ = false;
  return fd;
}

}  // namespace oak
