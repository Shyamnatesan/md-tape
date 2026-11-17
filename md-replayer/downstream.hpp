

#pragma once

#include "pacer/pacer.hpp"
#include "sources/packet_source.hpp"
#include "wire/udp.hpp"

namespace md_replayer {

template <sources::PacketSource Source, pacer::Pacer P> class DownstreamServer {
public:
  explicit DownstreamServer(Source &source, P &pacer, wire::UdpSender &tx)
      : source_(source), pacer_(pacer), tx_(tx) {}

  void Run() {
    while (auto packet = source_.next()) {
      pacer_.wait_until_due(packet->first_msg_ts);
      tx_.Send(packet->data);
      ++packets_sent_;
    }
  }

  std::uint64_t PacketsSent() const { return packets_sent_; }

private:
  Source &source_;
  P &pacer_;
  wire::UdpSender &tx_;
  std::uint64_t packets_sent_{0};
};
} // namespace md_replayer
