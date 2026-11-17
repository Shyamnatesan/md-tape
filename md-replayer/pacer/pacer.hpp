

#pragma once

#include <chrono>
#include <cstdint>
#include <thread>

namespace md_replayer::pacer {

template <typename P>
concept Pacer = requires(P &p, std::uint64_t ts) { p.wait_until_due(ts); };

// send the packets as they come in.
// this is empty, so the compiler will strip this logic away,
// and the call p.wait_until_due() gets stripped off
struct BurstPacer {
  void wait_until_due(std::uint64_t) noexcept {}
};

// simulate real time packet traffic. our pacer starts at the time, when the
// first packet is being sent. and for the subsequent packets, we calculate the
// elapsed time in nanoseconds, and the target time_point at which we should
// send this packet. and we simply wait until this target time, and send this
// packet. we wait by sleeping and then spinning to be very close to
// approximate.
class RealTimePacer {
public:
  explicit RealTimePacer(double speed = 1.0) : speed_(speed) {
    if (!(speed > 0.0)) {
      throw std::invalid_argument("speed must be greater than 0");
    }
  }

  void wait_until_due(std::uint64_t ts) {
    const auto now = Clock::now();
    if (!started_) {
      start_time_ = now;
      started_ = true;
      first_ts_ = ts;
      return;
    }

    // how long after the first packet, this one happened in the file.
    // if a timestamp ever goes backwards, treat it as 0 so the packet goes
    // out now.
    const std::uint64_t elapsed_ns = ts > first_ts_ ? ts - first_ts_ : 0;

    const auto target =
        start_time_ + std::chrono::nanoseconds(static_cast<std::int64_t>(
                          static_cast<double>(elapsed_ns) / speed_));

    wait_until(target);
  }

private:
  using Clock = std::chrono::steady_clock;

  constexpr static auto spin_window = std::chrono::microseconds(200);

  double speed_{1.0};
  std::uint64_t first_ts_{};
  Clock::time_point start_time_{};
  bool started_{false};

  void wait_until(Clock::time_point target) {

    // sleep until shortly before the target.
    // we dont sleep till target, because sleeping and waking up itself has
    // overhead, and is not instantaneuous. So, we sleep until we are closer to
    // the target, and then once we are closer, we wake up, and spin the
    // remaining gap.
    if (target - Clock::now() > spin_window) {
      std::this_thread::sleep_until(target - spin_window);
    }

    // spin until target
    while (Clock::now() < target) {
    }
  }
};

static_assert(Pacer<BurstPacer>);
static_assert(Pacer<RealTimePacer>);

} // namespace md_replayer::pacer
