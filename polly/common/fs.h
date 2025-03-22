// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_FS_H_
#define OAK_COMMON_FS_H_

#include <sys/types.h>
#include <string>

namespace oak {

// Return directory name of the path.
std::string DirectoryName(const std::string& path);

// Return file name of the path.
std::string Basename(const std::string& path);

// Return current working directory.
std::string GetCurrentDirectory();

// Return the canonicalized absolute path anme.
// Note: require the @path named file does exist.
std::string GetRealPath(const std::string& path);

// Creates an directory.
void CreateDirectory(const std::string& directory);

// Creates an directory recursively.
void CreateDirectoryRecursively(const std::string& directory);

// Changes current working directory to path where @path is inside.
void ChangeWorkDirectory(const std::string& directory);

// Tests for the existence of the path.
bool IsExists(const std::string& path);

class File {
 public:
  static const int kMode;

  // Open and create a file object for read only.
  static File MakeReadOnlyFile(const char* name);
  static File MakeReadOnlyFile(const std::string& name) {
    return MakeReadOnlyFile(name.c_str());
  }

  // Open and create a file object for write, if file already exists it will
  // be truncated to length 0.
  static File MakeWritableFile(const char* name);
  static File MakeWritableFile(const std::string& name) {
    return MakeWritableFile(name.c_str());
  }

  // Open and create a file object for write, if file already exists, before
  // write the file offset is positioned at the end, the modification of the
  // file offset and write operation are performed as a single atomic step.
  static File MakeAppendableFile(const char* name);
  static File MakeAppendableFile(const std::string& name) {
    return MakeAppendableFile(name.c_str());
  }

  // Open and create a file object for random access.
  static File MakeRandomAccessFile(const char* name);
  static File MakeRandomAccessFile(const std::string& name) {
    return MakeRandomAccessFile(name.c_str());
  }

  // Create a file object form an existing file descriptor, takes ownership
  // of the file descriptor if @owner is true.
  explicit File(int fd, bool owner = false) noexcept;

  // Create a file object by opening an file name.
  explicit File(const char* name, int flags, int mode = kMode);
  explicit File(const std::string& name, int flags, int mode = kMode);

  ~File();

  File(File&& other) noexcept;
  File& operator=(File&& other);

  // Close the file descriptor if owned, otherwise nothing to do.
  void Close();

  // Read up to @size bytes to @buffer from file object, the number of the
  // bytes read is returned, zero bytes indicates end of file.
  size_t Read(void* buffer, size_t size);

  // Writes up to @size bytes to file object from @buffer.
  size_t Write(const void* buffer, size_t size);
  size_t Write(const std::string& buffer) {
    return Write(buffer.c_str(), buffer.size());
  }

  // Reposition read/write offset, return offset location as measured in
  // bytes from the beginning of the file object.
  off_t Seek(off_t offset, int whence);

  // Synchronize state with storage device.
  // Note: A successful close does not guarantee that the data has been
  // sucessfully saved to disk, as the kernel uses the buffer cache to
  // defer writes, use Sync() to sure that the data is physically stored
  // on the underlying disk.
  void Sync();

  // Apply an exclusive advisory lock on file object.
  bool TryLock();

  // Remove an exclusive advisory lock on file object.
  void Unlock();

  // Releases and no longger owned the file descriptor.
  int Release() noexcept;

  // Return the file descriptor.
  int fd() const { return fd_; }

 private:
  File(File&) = delete;
  File& operator=(File const&) = delete;

  int fd_;
  bool owner_;
};

}  // namespace oak

#endif  // OAK_COMMON_FS_H_
