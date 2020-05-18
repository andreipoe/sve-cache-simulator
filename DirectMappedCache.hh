#pragma once

#include <vector>

#include "cache.hh"

class DirectMappedCache : public Cache {
  std::vector<CacheEntry> cache_lines;

 protected:
  virtual const CacheAddress split_address(const uint64_t address) const override;

 public:
  DirectMappedCache(const CacheConfig config);

  using Cache::touch;
  virtual CacheEvent touch(const uint64_t address) override;
};
