
#pragma once

#include "../mapped_file.hpp"
#include "packet_source.hpp"

#include "wire/constants.hpp"
#include "wire/endian.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace md_replayer::sources {

class ItchSource {
public:
  explicit ItchSource(const std::filesystem::path &path,
                      std::string_view session,
                      std::uint64_t max_gap_ns = 50'000)
      : file_(path), max_gap_ns_(max_gap_ns) {

    if (session.size() > session_.size()) {
      throw std::invalid_argument("session name must be at most 10 characters");
    }

    session_.fill(std::byte{' '});
    std::memcpy(session_.data(), session.data(), session.size());
  }

  std::optional<Packet> next() {

    using namespace wire;

    const auto data = file_.bytes();

    if (offset_ == data.size()) {
      return std::nullopt;
    }

    std::size_t used = sizeof(moldudp64::DownstreamHeader);

    std::uint16_t count = 0;
    std::uint64_t first_ts = 0;

    // we frame the messages in the packet, based on a fixed interval gap
    // between messages. this gap is the max_gap_ns_ member variable.
    //
    // eg: msg:timestamp, and assume timestamps are in seconds.
    // itch = [msg1: 1, msg2: 3, msg3: 7], so
    // msg1 arrived at timestamp 1, msg2 arrived at timestamp 3, and msg3 at 7.
    //
    // the first message always goes in the packet. And then we sequentially
    // check the message's timestamp with the previous message's timestamp, and
    // if they go beyond the allowed gap(max_gap_ns_), then whatever messages
    // are currently in the buffer_, goes in the packet, and this message goes
    // in the next packet.
    //
    // So, assume for max_gap is 3, initially packet is empty.
    // first msg goes in. so packet = [msg1]
    // second_msg.ts - first_msg.ts = 2,
    //      2 < 3, so goes in packet. packet = [msg1, msg2].
    // third_msg.ts - second_msg.ts = 4,
    //      4 > 3, so the current packet [msg1, msg2] is sent over the wire,
    //      and this third message goes in the next packet.
    //
    //
    // This is to simulate real-traffic like behavior, rather than always
    // sending almost full packets.
    //

    while (offset_ < data.size()) {
      const std::size_t remaining = data.size() - offset_;
      if (remaining < 2) {
        throw std::runtime_error(
            "broken ITCH file: can't read length at offset " +
            std::to_string(offset_));
      }

      const auto record = data.subspan(offset_);
      const std::uint16_t msg_len = LoadBE<std::uint16_t>(record);

      if (remaining - 2 < msg_len || msg_len < msg_ts_off + msg_ts_len) {
        throw std::runtime_error("broken ITCH file: bad message at offset " +
                                 std::to_string(offset_));
      }

      const auto message = record.subspan(2, msg_len);
      const std::uint64_t ts = ReadTimestamp(message);
      const std::size_t block_len = 2 + std::size_t{msg_len};

      if (count == 0) {
        // the first message always goes in
        first_ts = ts;
      } else {
        const std::uint64_t gap = ts > first_ts ? ts - first_ts : 0;
        const bool too_late = gap > max_gap_ns_;
        const bool too_big = used + block_len > buffer_.size();
        if (too_late || too_big) {
          // this message goes in the next packet, not this packet.
          break;
        }
      }

      std::memcpy(buffer_.data() + used, record.data(), block_len);
      used += block_len;
      offset_ += block_len;
      ++count;
    }

    moldudp64::DownstreamHeader hdr;
    hdr.session = session_;
    hdr.seq_num = ToBigEndian(next_seq_);
    hdr.msg_count = ToBigEndian(count);
    std::memcpy(buffer_.data(), &hdr, sizeof(hdr));

    next_seq_ += count;

    return Packet{std::span<const std::byte>(buffer_.data(), used), first_ts};
  }

private:
  // offset and length of timestamp in a message
  constexpr static std::size_t msg_ts_off = 5;
  constexpr static std::size_t msg_ts_len = 6;

  // memory mapped itch file
  MappedFile file_;

  // offset into the memory mapped file.
  std::size_t offset_{};

  // moldudp64 packet sequence tracker
  std::uint64_t next_seq_{1};

  // maximum gap between messages allowed in nanoseconds.
  std::uint64_t max_gap_ns_;

  // moldudp64 session, not really used here
  std::array<std::byte, wire::moldudp64::SessionLength> session_;

  // the raw packet buffer
  std::array<std::byte, wire::max_packet_size> buffer_{};

private:
  // timestamp in itch is 6 bytes, so we read this 6 byte timestamp into an 8
  // byte buffer, and then read it as a 8byte integer
  std::uint64_t ReadTimestamp(std::span<const std::byte> message) {
    std::array<std::byte, 8> padded{};
    std::memcpy(padded.data() + 2, message.data() + msg_ts_off, msg_ts_len);
    return wire::LoadBE<std::uint64_t>(padded);
  }
};

static_assert(PacketSource<ItchSource>);

} // namespace md_replayer::sources
