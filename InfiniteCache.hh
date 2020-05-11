#pragma once

#include <set>

#include "cache.hh"

class InfiniteCache : public Cache {
  std::set<long> addresses;

 public:
  InfiniteCache();

  using Cache::touch;
  virtual const CacheEvent touch(long address) override;
};
