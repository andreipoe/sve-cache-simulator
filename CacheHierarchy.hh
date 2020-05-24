#pragma once

#include <iostream>
#include <memory>

#include "cache.hh"


class CacheHierarchy {
  std::vector<std::unique_ptr<Cache>> levels;

  // TODO: support inclusive and exclusive caches

 public:
  CacheHierarchy(const std::vector<CacheConfig> cache_configs);
  CacheHierarchy(std::istream&& config_file);

  int nlevels() const;

  CacheType getType(int level) const;
  int getSize(int level) const;
  int getLineSize(int level) const;
  int getSetSize(int level) const;

  uint64_t getHits(int level) const;
  uint64_t getMisses(int level) const;
  uint64_t getTotalAccesses(int level) const;
  uint64_t getEvictions(int level) const;

  // TODO: implement hierarchy requests
};
