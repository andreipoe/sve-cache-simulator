#include "cache.hh"

#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"

Cache::Cache(const int size, const int line_size) : size(size), line_size(line_size) {}

Cache::Cache(const CacheConfig config) : size(config.size), line_size(config.line_size) {}

Cache::~Cache() {}

void Cache::touch(const std::vector<long> addresses) {
  for (auto const& a : addresses)
    touch(a);
}

const int Cache::getSize() const { return size; }
const int Cache::getLineSize() const { return line_size; }

const long Cache::getHits() const { return hits; }

const long Cache::getMisses() const { return misses; }

const long Cache::getTotalAccesses() const { return hits + misses; }

const CacheAddress Cache::split_address(const long address) const {
  throw NotImplementedException();
}

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
