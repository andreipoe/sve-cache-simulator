#pragma once

#include <set>

#include "cache.hh"

class InfiniteCache : public Cache {
  std::set<long> addresses;

 public:
  InfiniteCache();

  using Cache::touch;
  virtual CacheEvent touch(const uint64_t address) override;
};
