#pragma once

#include <set>

#include "cache.hh"

class InfiniteCache : public Cache {
  std::set<uint64_t> addresses;

 public:
  InfiniteCache(const std::shared_ptr<const Clock> clock);

  using Cache::touch;
  virtual CacheEvents touch(const CacheAddress& address) override;
  virtual CacheType getType() const override;
};
