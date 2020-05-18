#include "cache.hh"

#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"

static constexpr unsigned int log2(const uint64_t n) {
  return n == 1 ? 0 : 1 + log2(n >> 1);
}

// ------

CacheAddress::CacheAddress(uint64_t address, uint64_t cache_size,
                           unsigned int line_size) {
  const unsigned int block_bits { log2(line_size) };
  const unsigned int index_bits { log2(cache_size) - log2(line_size) };

  block = address & ((1 << block_bits) - 1);
  index = (address >> block_bits) & ((1 << index_bits) - 1);
  tag = address >> (block_bits + index_bits);
}

CacheAddress::CacheAddress(uint64_t address, const CacheConfig& config)
    : CacheAddress(address, config.size, config.line_size) {}

CacheAddress::CacheAddress(uint64_t address, const Cache& cache)
    : CacheAddress(address, cache.getSize(), cache.getLineSize()) {}

// ------

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
  return CacheAddress(address, *this);
}

void Cache::touch(const uint64_t address, const int size) {
  uint64_t next_address { address };
  int remaining_size { size };

  while (remaining_size > 0) {
    // Find the first cache line this request touches
    auto const& cache_address = split_address(next_address);
    touch(cache_address);

    // Skip over the remaining bytes in this same cache line
    const unsigned int covered_bytes = line_size - cache_address.block;
    remaining_size -= covered_bytes;
    next_address += covered_bytes;
  }
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
