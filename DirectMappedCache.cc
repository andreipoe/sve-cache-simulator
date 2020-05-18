#include "DirectMappedCache.hh"

DirectMappedCache::DirectMappedCache(const CacheConfig config) : Cache(config) {
  cache_lines.resize(size / block_bits, { 0, false });
}

CacheEvent DirectMappedCache::touch(const uint64_t address) {
  auto address_split = split_address(address);
  auto cl_index = address_split.index;

  auto cached_element = cache_lines[cl_index];
  CacheEvent event;

  if (cached_element.valid && cached_element.tag == address_split.tag) {
    hits++;
    event = CacheEvent::Hit;
  } else {
    misses++;
    event = CacheEvent::Miss;
  }

  cache_lines[cl_index] = { address_split.tag, true };

  return event;
}
