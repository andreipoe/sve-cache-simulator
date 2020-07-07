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

/* Returns a a liftime map for the elements still in the cache */
std::unique_ptr<std::map<uint64_t, uint64_t>> DirectMappedCache::getActiveLifetimes()
    const {
  auto active_lifetimes = std::make_unique<std::map<uint64_t, uint64_t>>();
  for (auto const& line : cache_lines)
    if (line.valid) (*active_lifetimes)[clock_->current_cycle() - line.loaded_at]++;

  return active_lifetimes;
}

CacheType DirectMappedCache::getType() const { return CacheType::DirectMapped; }
