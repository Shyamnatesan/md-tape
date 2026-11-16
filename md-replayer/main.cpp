

#include "packet_source.hpp"
#include "sources/itch.hpp"
#include "wire/udp.hpp"

#include <cstddef>
#include <filesystem>

using namespace md_replayer;
using namespace md_replayer::sources;

using namespace wire;

template <PacketSource Source>
void replay(Source &source, wire::UdpSender &tx) {
  while (auto packet = source.next()) {
    tx.Send(packet->data);
  }
}

int main() {

  std::filesystem::path path{"data/07302019.NASDAQ_ITCH50"};

  ItchSource source{path};
  UdpSender tx{"127.0.0.1", "9000"};

  replay<ItchSource>(source, tx);
}
