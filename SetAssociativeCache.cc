#include "SetAssociativeCache.hh"

SetAssociativeCache::SetAssociativeCache(const CacheConfig config,
                                         const std::shared_ptr<const Clock> clock)
    : Cache(config, clock), cache_sets(size / (line_size * set_size)) {

  for (size_t i = 0; i < cache_sets.size(); i++)
    cache_sets[i] = std::vector<CacheEntry>(set_size, CacheEntry {});
}

CacheEvents SetAssociativeCache::touch(const CacheAddress& address) {
  CacheEntry* hit { nullptr };
  CacheEvents events {};

  CacheEntry* oldest = &cache_sets[address.index][0];
  uint64_t max_age   = oldest->age;
  for (CacheEntry& cache_line : cache_sets[address.index]) {
    cache_line.age += 1;
    if (cache_line.tag == address.tag && cache_line.valid) hit = &cache_line;
    if (cache_line.age > max_age) {
      max_age = cache_line.age;
      oldest  = &cache_line;
    }
  }

  if (hit) {
    hits++;
    events.hits++;
  } else {
    if (oldest->valid) {
      evictions++;
      events.evictions++;
      log_eviction(oldest->loaded_at);
    }
    misses++;
    events.misses++;

    oldest->set(address.tag, clock_->current_cycle());
  }

  return events;
}

/* Returns a a liftime map for the elements still in the cache */
std::unique_ptr<std::map<uint64_t, uint64_t>> SetAssociativeCache::getActiveLifetimes()
    const {
  auto active_lifetimes = std::make_unique<std::map<uint64_t, uint64_t>>();
  for (auto const& set : cache_sets)
    for (auto const& line : set)
      if (line.valid) (*active_lifetimes)[clock_->current_cycle() - line.loaded_at]++;

  return active_lifetimes;
}

CacheType SetAssociativeCache::getType() const { return CacheType::SetAssociative; }
