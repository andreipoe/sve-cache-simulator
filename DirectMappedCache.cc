#include "DirectMappedCache.hh"

DirectMappedCache::DirectMappedCache(const CacheConfig config,
                                     const std::shared_ptr<const Clock> clock)
    : Cache(config, clock), cache_lines(size / line_size, CacheEntry {}) { }

CacheEvents DirectMappedCache::touch(const CacheAddress& cache_address) {
  auto cached_element = cache_lines[cache_address.index];
  CacheEvents events {};

  if (cached_element.valid && cached_element.tag == cache_address.tag) {
    hits++;
    events.hits++;
  } else {
    if (cached_element.valid) {
      evictions++;
      events.evictions++;
      log_eviction(cached_element.loaded_at);
    }
    misses++;
    events.misses++;
  }

  cache_lines[cache_address.index].set(cache_address.tag, clock_->current_cycle());

  return events;
}

CacheType DirectMappedCache::getType() const { return CacheType::DirectMapped; }
