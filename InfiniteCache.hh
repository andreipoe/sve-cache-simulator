#pragma once

#include <set>

#include "cache.hh"

class InfiniteCache : public Cache {
  std::set<uint64_t> addresses;

 public:
  InfiniteCache();

  using Cache::touch;
  virtual CacheEvent touch(const uint64_t address) override;
};
