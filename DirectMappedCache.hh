#pragma once

#include <vector>

#include "cache.hh"

class DirectMappedCache : public Cache {
  std::vector<CacheEntry> cache_lines;

 protected:
  virtual const CacheAddress split_address(long address) const override;

 public:
  DirectMappedCache(CacheConfig config);

  using Cache::touch;
  virtual const CacheEvent touch(long address) override;
};
