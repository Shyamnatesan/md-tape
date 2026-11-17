

#pragma once

#include <cstddef>
#include <fcntl.h>

#include <filesystem>
#include <span>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace md_replayer {

namespace detail {

struct Fd {
  int fd_{-1};

  ~Fd() {
    if (fd_ >= 0) {
      ::close(fd_);
    }
  }
};

} // namespace detail

using namespace detail;

// right now, this MappedFile should only be used for reading a file.
// not for writing.
class MappedFile {
public:
  MappedFile(const std::filesystem::path &path) {
    Fd file{::open(path.c_str(), O_RDONLY)};
    if (file.fd_ < 0) {
      throw std::runtime_error("open failed");
    }

    struct stat info{};
    if (::fstat(file.fd_, &info) < 0) {
      throw std::runtime_error("fstat failed");
    }

    const std::size_t size = static_cast<std::size_t>(info.st_size);
    if (size == 0) {
      return;
    }

    void *addr = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, file.fd_, 0);
    if (addr == MAP_FAILED) {
      throw std::runtime_error("mmap failed");
    }

    madvise(addr, size, MADV_SEQUENTIAL);

    bytes_ = {static_cast<std::byte *>(addr), size};
    size_ = size;
  }

  std::span<const std::byte> bytes() const noexcept { return bytes_; }

  ~MappedFile() {
    if (!bytes_.empty()) {
      ::munmap(bytes_.data(), bytes_.size());
    }
    size_ = 0;
  }

  MappedFile(const MappedFile &) = delete;
  MappedFile &operator=(const MappedFile &) = delete;

  MappedFile(MappedFile &&) = delete;
  MappedFile &operator=(MappedFile &&) = delete;

private:
  std::span<std::byte> bytes_{};
  std::size_t size_{};
};

} // namespace md_replayer
