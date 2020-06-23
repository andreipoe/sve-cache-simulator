#pragma once

#include <cstdint>

class Clock {
  uint64_t current_cycle_ { 0 };

 public:
  uint64_t current_cycle() const;
  void tick();
};
