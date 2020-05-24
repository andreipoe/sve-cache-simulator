#pragma once

#include <iostream>
#include <memory>

#include "cache.hh"


class CacheHierarchy {
  std::vector<std::unique_ptr<Cache>> levels;

  // TODO: support inclusive and exclusive caches

 public:
  CacheHierarchy(std::vector<CacheConfig> cache_configs);
  CacheHierarchy(std::istream&& config_file);

  int nlevels() const;

  // TODO: figure out a way to get the type
  // CacheType getType() const;
  int getSize(int level) const;
  int getLineSize(int level) const;
  int getSetSize(int level) const;

  // TODO: implement hierarchy stats
  // uint64_t getHits() const;
  // uint64_t getMisses() const;
  // uint64_t getTotalAccesses() const;
  // uint64_t getEvictions() const;

  // TODO: implement hierarchy requests
};
