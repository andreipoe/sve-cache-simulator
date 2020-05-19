#pragma once

#include <vector>

#include "cache.hh"

class SetAssociativeCache : public Cache {

  using CacheSet = std::vector<CacheEntry>;
  std::vector<CacheSet> cache_sets;

 public:
  SetAssociativeCache(const CacheConfig config);

  using Cache::touch;
  virtual CacheEvent touch(const CacheAddress& address) override;
};