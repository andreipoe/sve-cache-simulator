#pragma once

#include <vector>

#include "cache.hh"

class DirectMappedCache : public Cache {
  std::vector<CacheEntry> cache_lines;

  /* Returns a a liftime map for the elements still in the cache */
  virtual std::unique_ptr<std::map<uint64_t, uint64_t>> getActiveLifetimes()
      const override;

 public:
  DirectMappedCache(const CacheConfig config, const std::shared_ptr<const Clock> clock);

  using Cache::touch;
  virtual CacheEvents touch(const CacheAddress& address) override;
  virtual CacheType getType() const override;
};
