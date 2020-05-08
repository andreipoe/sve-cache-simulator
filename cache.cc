#include "cache.hh"

Cache::Cache() {}
Cache::~Cache() {}

const long Cache::getHits() const { return hits; }

const long Cache::getMisses() const { return misses; }

const long Cache::getTotalAccesses() const { return hits + misses; }
