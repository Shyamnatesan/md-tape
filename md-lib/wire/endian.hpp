

#pragma once

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <span>

namespace wire {

// TODO: std::byteswap is fine, but only works on latest compilers.
// maybe manual bit-shifting for cross-platform support ?

template <std::unsigned_integral T> T LoadBE(std::span<const std::byte> bytes) {
  assert(bytes.size() >= sizeof(T));

  T value;

  std::memcpy(&value, bytes.data(), sizeof(value));

  if constexpr (std::endian::native == std::endian::little) {
    value = std::byteswap(value);
  }

  return value;
}

template <std::unsigned_integral T> T ToBigEndian(T value) {
  if constexpr (std::endian::native == std::endian::little) {
    return std::byteswap(value);
  }
  return value;
}

template <std::unsigned_integral T>
void StoreBE(std::span<std::byte> out, T value) {
  if constexpr (std::endian::native == std::endian::little) {
    value = std::byteswap(value);
  }
  std::memcpy(out.data(), &value, sizeof(value));
}

} // namespace wire
