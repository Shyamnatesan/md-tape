
#pragma once

#include "../mapped_file.hpp"
#include "../packet_source.hpp"
#include <cstddef>
#include <filesystem>
#include <optional>

namespace md_replayer::sources {

class ItchSource {
public:
  explicit ItchSource(const std::filesystem::path &path) : file_(path) {}

  std::optional<Packet> next() {
    // Simplified logic:
    //
    // what should next do here ?
    //
    // TODO:
    //  1. research the max packet size. (maybe have this constant in md-lib as
    //  well)
    //  2. create a buffer on the stack for this size
    //  3. write the moldudp struct in md-lib
    //  4. write the endian readers and writers helpers in md-lib
    //  5. so every itch message has a timestamp.
    //      eg: msg1.t1, msg2.t2, msg3.t3
    //          here the messages are ordered by time. so t1 < t2 < t3, ... tn
    //  6. we have two options here.
    //      a. accumulate the messages until the packet fills up, and send the
    //      packet. (receiver always receives an almost full packet). (OR)
    //
    //      b. accumulate the messages based on max gap in nanoseconds.
    //         eg: assume the example data. format: msg:timestamp(in seconds)
    //             itch file = [msg1:1, msg2:3, msg3:7]
    //             so here, message 1 arrived at timestamp 1
    //                      message 2 arrived at timestamp 3
    //                      and message 3 arrived at timsetamp 7.
    //             so, assuming we set a max_gap as 3 seconds,
    //
    //              we accumulate the first message into the packet.
    //              and then when we get the next message, we check if the
    //              difference in timestamp between this message and the
    //              previous messsage is less than the max_gap, if so, we send
    //              them at once. or else, we send the current packet as it is,
    //              and send these in the next packet.
    //
    //            So, based on this logic, our intial packet buffer is empty[].
    //            we accumulate the very first message. packet = [msg1]
    //            message 2 arrived at timestamp 3, so the gap is 2 seconds, so
    //            we accumulate this as well. packet = [msg1, msg2]. next,
    //            message 3 arrived at timestamp 7, so the gap is 5 seconds, so
    //            we do not take this now. we send the current packet as it is.
    //            ie [msg1, msg2], and send the msg3 in the next packet.
    //
    //        This kind of simulates real-time traffic, rather than always
    //        sending packets with full data.
    //

    return std::nullopt;
  }

private:
  MappedFile file_;
  std::size_t offset_{};
};

static_assert(PacketSource<ItchSource>);

} // namespace md_replayer::sources
