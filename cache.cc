#include "cache.hh"

#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"
#include "SetAssociativeCache.hh"

static constexpr unsigned int nbits(const uint64_t n) {
  return n == 1 ? 0 : 1 + nbits(n >> 1);
}

// ------

CacheEvents& CacheEvents::operator+=(const CacheEvents& rhs) {
  this->hits += rhs.hits;
  this->misses += rhs.misses;
  this->evictions += rhs.evictions;

  return *this;
}

bool CacheEvents::hit() const { return misses == 0; }

// ------

CacheAddress::CacheAddress(uint64_t address, uint64_t cache_size, int line_size,
                           int set_size) {
  const unsigned int block_bits { nbits(line_size) };
  const unsigned int index_bits { nbits(cache_size) - nbits(line_size) -
                                  nbits(set_size) };

  block = address & ((1 << block_bits) - 1);
  index = (address >> block_bits) & ((1 << index_bits) - 1);
  tag   = address >> (block_bits + index_bits);
}

CacheAddress::CacheAddress(uint64_t address, const CacheConfig& config)
    : CacheAddress(address, config.size, config.line_size, config.set_size) {}

CacheAddress::CacheAddress(uint64_t address, const Cache& cache)
    : CacheAddress(address, cache.getSize(), cache.getLineSize(), cache.getSetSize()) {}

// ------

CacheEntry::CacheEntry(uint64_t tag) : tag(tag), valid(true) {}

// ------

Cache::Cache(const uint64_t size, const int line_size, const int set_size)
    : size(size), line_size(line_size), set_size(set_size) {
  if (size % line_size != 0)
    throw std::invalid_argument("Line size does not divide cache size");
  if (size % set_size != 0)
    throw std::invalid_argument("Set size does not divide cache size");
  if ((size & (size - 1)) != 0)
    throw std::invalid_argument("Cache size is not a power of 2");
}

Cache::Cache(const CacheConfig config)
    : Cache(config.size, config.line_size, config.set_size) {}

Cache::~Cache() {}

const CacheAddress Cache::split_address(const uint64_t address) const {
  return CacheAddress(address, *this);
}

CacheEvents Cache::touch(const uint64_t address, const int size) {
  CacheEvents events {};
  uint64_t next_address { address };
  int remaining_size { size };

  while (remaining_size > 0) {
    // Find the first cache line this request touches
    auto const& cache_address = split_address(next_address);
    events += touch(cache_address);

    // Skip over the remaining bytes in this same cache line
    const unsigned int covered_bytes = line_size - cache_address.block;
    remaining_size -= covered_bytes;
    next_address += covered_bytes;
  }

  return events;
}

CacheEvents Cache::touch(const SizedAccess access) {
  return touch(access.address, access.size);
}

CacheEvents Cache::touch(MemoryRequest request) {
  return touch(request.address, request.size);
}

CacheEvents Cache::touch(const std::vector<uint64_t> addresses) {
  CacheEvents events {};
  for (auto const& a : addresses) events += touch(a);
  return events;
}

CacheEvents Cache::touch(const std::vector<CacheAddress> addresses) {
  CacheEvents events {};
  for (auto const& a : addresses) events += touch(a);
  return events;
}

CacheEvents Cache::touch(const std::vector<SizedAccess> accesses) {
  CacheEvents events {};
  for (auto const& a : accesses) events += touch(a);
  return events;
}

CacheEvents Cache::touch(const std::vector<MemoryRequest> requests) {
  CacheEvents events {};
  for (auto const& r : requests) events += touch(r);
  return events;
}

uint64_t Cache::getSize() const { return size; }
int Cache::getLineSize() const { return line_size; }
int Cache::getSetSize() const { return set_size; }

uint64_t Cache::getHits() const { return hits; }
uint64_t Cache::getMisses() const { return misses; }
uint64_t Cache::getTotalAccesses() const { return hits + misses; }
uint64_t Cache::getEvictions() const { return evictions; }

std::unique_ptr<Cache> Cache::make_cache(const CacheConfig config) {
  switch (config.type) {
    case CacheType::Infinite:
      return std::make_unique<InfiniteCache>();
    case CacheType::DirectMapped:
      return std::make_unique<DirectMappedCache>(config);
    case CacheType::SetAssociative:
      return std::make_unique<SetAssociativeCache>(config);
    default:
      throw std::invalid_argument("Unknown cache type");
  }
}

NotImplementedException::NotImplementedException()
    : std::logic_error("Not implemented") {}
