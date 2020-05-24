#pragma once

#include <vector>

#include "cache.hh"

class DirectMappedCache : public Cache {
  std::vector<CacheEntry> cache_lines;

 public:
  DirectMappedCache(const CacheConfig config);

  using Cache::touch;
  virtual CacheEvent touch(const CacheAddress& address) override;
  virtual CacheType getType() const override;
};
