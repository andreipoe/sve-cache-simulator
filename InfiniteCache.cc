#include "InfiniteCache.hh"

InfiniteCache::InfiniteCache() : Cache(static_cast<uint64_t>(1) << 48, 64) {}

CacheEvent InfiniteCache::touch(const CacheAddress& cache_address) {
  auto cached_element = addresses.find(cache_address.index);
  CacheEvent event;

  if (cached_element != addresses.end()) {
    hits++;
    event = CacheEvent::Hit;
  } else {
    misses++;
    event = CacheEvent::Miss;
  }

  addresses.insert(cache_address.index);
  return event;
}

CacheType InfiniteCache::getType() const { return CacheType::Infinite; }
