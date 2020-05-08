#pragma once

#include <set>

#include "cache.hh"

class InfiniteCache : public Cache {
  std::set<long> addresses;

 public:
  InfiniteCache();

  virtual const CacheEvent touch(long address) override;
};
