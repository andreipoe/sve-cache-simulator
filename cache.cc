#include "cache.hh"

#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"

constexpr unsigned int log2(const uint64_t n) { return n < 2 ? n : 1 + log2(n >> 1); }

Cache::Cache(const uint64_t size, const int line_size)
    : size(size),
      line_size(line_size),
      block_bits(log2(line_size)),
      index_bits(log2(size) - log2(line_size)) {}

Cache::Cache(const CacheConfig config)
    : size(config.size),
      line_size(config.line_size),
      block_bits(log2(config.line_size)),
      index_bits(log2(config.size) - log2(config.line_size)) {}

Cache::~Cache() {}

const CacheAddress Cache::split_address(const uint64_t address) const {
  int block = address & ((1 << block_bits) - 1);
  int index = (address >> block_bits) & ((1 << index_bits) - 1);
  uint64_t tag = address >> (block_bits + index_bits);

  return { tag, index, block };
}

void Cache::touch(const std::vector<uint64_t> addresses) {
  for (auto const& a : addresses)
    touch(a);
}

int Cache::getSize() const { return size; }
int Cache::getLineSize() const { return line_size; }

uint64_t Cache::getHits() const { return hits; }

uint64_t Cache::getMisses() const { return misses; }

uint64_t Cache::getTotalAccesses() const { return hits + misses; }

std::unique_ptr<Cache> Cache::make_cache(const CacheConfig config) {
  switch (config.type) {
    case CacheType::Infinite:
      return std::make_unique<InfiniteCache>();
    case CacheType::DirectMapped:
      return std::make_unique<DirectMappedCache>(config);
    default:
      throw std::invalid_argument("Unknown cache type");
  }
}


NotImplementedException::NotImplementedException()
    : std::logic_error("Not implemented") {}
