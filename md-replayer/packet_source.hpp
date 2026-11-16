

#pragma once

#include <concepts>
#include <optional>
#include <span>

namespace md_replayer::sources {

struct Packet {
  std::span<const std::byte> data;
};

// we have multiple sources.
// so, each source, must expose a next() method which returns an optional packet
template <typename Source>
concept PacketSource = requires(Source s) {
  { s.next() } -> std::same_as<std::optional<Packet>>;
};

} // namespace md_replayer::sources
