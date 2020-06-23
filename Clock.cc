#include "Clock.hh"

uint64_t Clock::current_cycle() const {
  return current_cycle_;
}

void Clock::tick() {
  current_cycle_++;
}
