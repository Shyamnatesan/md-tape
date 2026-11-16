

#pragma once

#include <cstddef>
#include <fcntl.h>

#include <filesystem>
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

    data_ = static_cast<std::byte *>(addr);
    size_ = size;
  }

  std::byte *data() const { return data_; }

  std::size_t size() const { return size_; }

  ~MappedFile() {
    if (data_ != nullptr) {
      ::munmap(data_, size_);
    }
    data_ = nullptr;
    size_ = 0;
  }

  MappedFile(const MappedFile &) = delete;
  MappedFile &operator=(const MappedFile &) = delete;

  MappedFile(MappedFile &&) = delete;
  MappedFile &operator=(MappedFile &&) = delete;

private:
  std::byte *data_{nullptr};
  std::size_t size_{};
};

} // namespace md_replayer
