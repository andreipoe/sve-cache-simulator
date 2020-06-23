#pragma once

#include <vector>

#include "cache.hh"

class DirectMappedCache : public Cache {
  std::vector<CacheEntry> cache_lines;

 public:
  DirectMappedCache(const CacheConfig config, const std::shared_ptr<const Clock> clock);

  using Cache::touch;
  virtual CacheEvents touch(const CacheAddress& address) override;
  virtual CacheType getType() const override;
};
