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

/* Returns a a liftime map for the elements still in the cache */
std::unique_ptr<std::map<uint64_t, uint64_t>> InfiniteCache::getActiveLifetimes() const {
  throw std::logic_error("Infinite caches don't implement lifetimes");
  return std::make_unique<std::map<uint64_t,uint64_t>>();
}

CacheType InfiniteCache::getType() const { return CacheType::Infinite; }
