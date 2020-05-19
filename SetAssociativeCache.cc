#include "SetAssociativeCache.hh"

SetAssociativeCache::SetAssociativeCache(const CacheConfig config)
    : Cache(config),
      cache_sets(size / (line_size * set_size)) {

  for (size_t i = 0; i < cache_sets.size(); i++)
    cache_sets[i] = std::vector<CacheEntry>(set_size, CacheEntry{});
}

CacheEvent SetAssociativeCache::touch(const CacheAddress& address) {
  uint64_t max_age { 0 };
  CacheEntry *hit { nullptr }, *oldest { nullptr };

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
    return CacheEvent::Hit;
  } else {
    misses++;
    *oldest = { address.tag };
    return CacheEvent::Miss;
  }
}

