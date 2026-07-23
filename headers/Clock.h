#ifndef CLOCK_H
#define CLOCK_H

#include <cstdint>

// std::chrono-backed replacements for SDL_GetTicks()/SDL_Delay(), keeping the
// same uint32 monotonic-milliseconds contract so call sites barely change.
uint32_t nowMs();
void sleepMs(uint32_t ms);

#endif
