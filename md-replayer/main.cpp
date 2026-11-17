

#include "downstream.hpp"
#include "sources/itch.hpp"
#include "wire/udp.hpp"

#include <cstddef>
#include <filesystem>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::fprintf(stderr, "usage: %s <itch-file> [--realtime [speed]]\n",
                 argv[0]);
    return 1;
  }

  md_replayer::sources::ItchSource source{argv[1], "SESSION001"};
  wire::UdpSender tx{"127.0.0.1", "9000"};

  const bool realtime = argc >= 3 && std::string_view{argv[2]} == "--realtime";

  std::uint64_t sent = 0;
  if (realtime) {
    const double speed = argc >= 4 ? std::stod(argv[3]) : 1.0;
    md_replayer::pacer::RealTimePacer pacer{speed};
    md_replayer::DownstreamServer server{source, pacer, tx};
    server.Run();
    sent = server.PacketsSent();
  } else {
    md_replayer::pacer::BurstPacer pacer;
    md_replayer::DownstreamServer server{source, pacer, tx};
    server.Run();
    sent = server.PacketsSent();
  }

  std::printf("sent %llu packets\n", static_cast<unsigned long long>(sent));
}
