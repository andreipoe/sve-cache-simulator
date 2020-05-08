#include "InfiniteCache.hh"

InfiniteCache::InfiniteCache() {}

const CacheEvent InfiniteCache::touch(long address) {
  auto cached_element = addresses.find(address);
  CacheEvent event;

  if (cached_element != addresses.end()) {
    hits++;
    event = CacheEvent::Hit;
  } else {
    misses++;
    event = CacheEvent::Miss;
  }

  addresses.insert(address);
  return event;
}
