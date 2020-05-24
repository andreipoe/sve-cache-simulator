#include "DirectMappedCache.hh"

DirectMappedCache::DirectMappedCache(const CacheConfig config)
    : Cache(config), cache_lines(size / line_size, CacheEntry {}) {}

CacheEvent DirectMappedCache::touch(const CacheAddress& cache_address) {
  auto cached_element = cache_lines[cache_address.index];
  CacheEvent event;

  if (cached_element.valid && cached_element.tag == cache_address.tag) {
    hits++;
    event = CacheEvent::Hit;
  } else {
    if (cached_element.valid) evictions++;
    misses++;
    event = CacheEvent::Miss;
  }

  cache_lines[cache_address.index] = { cache_address.tag };

  return event;
}

CacheType DirectMappedCache::getType() const { return CacheType::DirectMapped; }
