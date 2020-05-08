#include "cache.hh"

Cache::Cache() {}
Cache::~Cache() {}

const long Cache::getHits() const { return hits; }

const long Cache::getMisses() const { return misses; }

const long Cache::getTotalAccesses() const { return hits + misses; }

const CacheAddress Cache::split_address(long address) const {
  throw NotImplementedException();
}

NotImplementedException::NotImplementedException()
    : std::logic_error("Not implemented") {}
