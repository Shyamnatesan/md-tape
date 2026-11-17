

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace wire {

// TODO: make this a command line arg
constexpr std::size_t max_packet_size = 1472;

namespace moldudp64 {

constexpr std::size_t SessionLength = 10;

#pragma pack(push, 1)
struct DownstreamHeader {
  std::array<std::byte, SessionLength> session;
  std::uint64_t seq_num;
  std::uint16_t msg_count;
};
#pragma pack(pop)

static_assert(sizeof(DownstreamHeader) == 20);

} // namespace moldudp64

} // namespace wire
