

#pragma once

#include <concepts>
#include <cstdint>
#include <optional>
#include <span>

namespace md_replayer::sources {

struct Packet {
  // the raw packet data.
  std::span<const std::byte> data;

  // the timestamp of the first message in the packet.
  // used for simulating traffic
  std::uint64_t first_msg_ts;
};

// we have multiple sources. so, each source, must expose a next() method which
// returns an optional packet
template <typename Source>
concept PacketSource = requires(Source s) {
  { s.next() } -> std::same_as<std::optional<Packet>>;
};

} // namespace md_replayer::sources
