

#pragma once

#include <cstddef>
#include <netdb.h>
#include <span>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace wire {

namespace detail {

enum class Role {
  Sender,
  Listener,
};

class Socket {
public:
  explicit Socket(int fd = -1) : fd_(fd) {}

  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  Socket(Socket &&) = delete;
  Socket &operator=(Socket &&) = delete;

  ~Socket() {
    if (fd_ >= 0) {
      ::close(fd_);
    }
    fd_ = -1;
  }

  int fd() const { return fd_; }

private:
  int fd_{-1};
};

inline Socket MakeUdpSocket(const char *host, const std::string &port,
                            Role role) {
  addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  if (role == Role::Listener) {
    hints.ai_flags = AI_PASSIVE;
  }

  addrinfo *servinfo = nullptr;
  int rv = getaddrinfo(host, port.c_str(), &hints, &servinfo);
  if (rv != 0) {
    throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(rv));
  }

  int fd = -1;
  for (addrinfo *p = servinfo; p != nullptr; p = p->ai_next) {
    fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (fd == -1) {
      continue;
    }

    int ok = (role == Role::Sender) ? ::connect(fd, p->ai_addr, p->ai_addrlen)
                                    : ::bind(fd, p->ai_addr, p->ai_addrlen);
    if (ok == 0) {
      break;
    }

    ::close(fd);
    fd = -1;
  }

  freeaddrinfo(servinfo);

  if (fd == -1) {
    throw std::runtime_error("could not set up UDP socket");
  }
  return Socket(fd);
}

} // namespace detail

using namespace detail;

class UdpSender {
public:
  UdpSender(const std::string &host, const std::string &port)
      : sock_(MakeUdpSocket(host.c_str(), port, Role::Sender)) {}

  bool Send(std::span<const std::byte> data) {
    ssize_t n = ::send(sock_.fd(), data.data(), data.size(), 0);
    return n == static_cast<ssize_t>(data.size());
  }

private:
  Socket sock_;
};

class UdpListener {
public:
  explicit UdpListener(const std::string &port, const std::string &host = "")
      : sock_(MakeUdpSocket(host.empty() ? nullptr : host.c_str(), port,
                            Role::Listener)) {}

  ssize_t Receive(std::span<std::byte> buf) {
    return ::recv(sock_.fd(), buf.data(), buf.size(), 0);
  }

private:
  Socket sock_;
};

} // namespace wire
