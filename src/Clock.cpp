#include "Clock.h"
#include <chrono>
#include <thread>

namespace
{
using SteadyClock = std::chrono::steady_clock;
const SteadyClock::time_point kStart = SteadyClock::now();
} // namespace

uint32_t nowMs()
{
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(SteadyClock::now() - kStart).count();
}

void sleepMs(uint32_t ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
