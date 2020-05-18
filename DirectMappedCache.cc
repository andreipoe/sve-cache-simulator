#include "DirectMappedCache.hh"

// TODO: make these parameters configurable
const int cache_size = 32 * 1024;  // 32 KB
const int block_size = 6;          // 64-byte cache lines
const int index_size = 9;

DirectMappedCache::DirectMappedCache(const CacheConfig config) : Cache(config) {
  cache_lines.resize(cache_size / block_size, { 0, false });
}

const CacheAddress DirectMappedCache::split_address(const long address) const {
  int block = address & ((1 << block_size) - 1);
  int index = (address >> block_size) & ((1 << index_size) - 1);
  long tag = address >> (block_size + index_size);

  return { tag, index, block };
}

CacheEvent DirectMappedCache::touch(const long address) {
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
