#include "InfiniteCache.hh"

#include <limits>

InfiniteCache::InfiniteCache() : Cache(std::numeric_limits<uint64_t>::max(), 64) {}

CacheEvent InfiniteCache::touch(const uint64_t address) {
  auto const& cache_address = split_address(address);
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
