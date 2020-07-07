#pragma once

#include <set>

#include "cache.hh"

class InfiniteCache : public Cache {
  std::set<uint64_t> addresses;

  /* Returns a a liftime map for the elements still in the cache */
  virtual std::unique_ptr<std::map<uint64_t, uint64_t>> getActiveLifetimes() const override;

 public:
  InfiniteCache(const std::shared_ptr<const Clock> clock);

  using Cache::touch;
  virtual CacheEvents touch(const CacheAddress& address) override;
  virtual CacheType getType() const override;
};
