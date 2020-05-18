#pragma once

#include <vector>

#include "cache.hh"

class DirectMappedCache : public Cache {
  std::vector<CacheEntry> cache_lines;

 protected:
  virtual const CacheAddress split_address(const long address) const override;

 public:
  DirectMappedCache(const CacheConfig config);

  using Cache::touch;
  virtual CacheEvent touch(const long address) override;
};
