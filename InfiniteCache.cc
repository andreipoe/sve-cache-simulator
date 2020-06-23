#include "InfiniteCache.hh"

InfiniteCache::InfiniteCache(const std::shared_ptr<const Clock> clock) : Cache(static_cast<uint64_t>(1) << 48, 64, 1, clock) {}

CacheEvents InfiniteCache::touch(const CacheAddress& cache_address) {
  auto cached_element = addresses.find(cache_address.index);
  CacheEvents events {};

  if (cached_element != addresses.end()) {
    hits++;
    events.hits++;
  } else {
    misses++;
    events.misses++;
  }

  addresses.insert(cache_address.index);
  return events;
}

CacheType InfiniteCache::getType() const { return CacheType::Infinite; }
