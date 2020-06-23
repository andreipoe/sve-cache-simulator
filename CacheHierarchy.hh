#pragma once

#include <iostream>
#include <memory>

#include "cache.hh"


struct BundleStats {
  uint64_t times_encountered { 0 }, total_ops { 0 };
};

class CacheHierarchy {
  // TODO: support inclusive and exclusive caches

  /* A 0-indexed list of cache levels (Ln is `levels[n-1]`) */
  std::vector<std::unique_ptr<Cache>> levels;

  /* The cache traffic, in bytes, between each level and the one above.
   * `traffic[0]` is the total amount of data requested from this hierarchy
   * `traffic[nlevels()]` is the total amount of data transferred between this hierarchy
   * and main memory */
  std::vector<uint64_t> traffic;

  /* The scatter/gather bundles encountered, as a mapping from PC to number of separate
   * ops */
  std::map<uint64_t, BundleStats> bundles;

  /* A counter of how many requests this hierarchy has serviced so far. Each cache level
   can read this shared counter, but only the hierarchy can cause it to tick */
  const std::shared_ptr<Clock> clock_;

  void constuctor_common_();

 public:
  CacheHierarchy(const std::vector<CacheConfig>& cache_configs);
  CacheHierarchy(std::istream&& config_file);

  /* Parameters */
  int nlevels() const;
  CacheType getType(int level) const;
  int getSize(int level) const;
  int getLineSize(int level) const;
  int getSetSize(int level) const;

  /* Returns the current value shown by the clock */
  uint64_t current_cycle() const;

  /* Stats */
  uint64_t getHits(int level) const;
  uint64_t getMisses(int level) const;
  uint64_t getTotalAccesses(int level) const;
  uint64_t getEvictions(int level) const;

  /* TODO: it may be useful to add a metric for "useful" cache traffic, counting how much
   data was requested and had to be fetched, as opposed to how much was actually fetched
   due to cache line restrictions  */
  /* Get the traffic, in bytes, between the given level and the one above */
  uint64_t getTraffic(int from_level) const;

  /* Get a mapping from scatter/gather PCs to number of accesses executed */
  std::map<uint64_t, BundleStats> getBundleOps() const;

  /* Returns a histogram of how long cache lines last in the given cache level before being evicted */
  const std::map<uint64_t,uint64_t> getLifetimes(int level) const;


  /* Accesses*/
  /* Run a single request through the cache hierarchy */
  void touch(uint64_t address, int size = 1);

  /* Run a single request through the cache hierarchy */
  void touch(SizedAccess access);

  /* Run a single request through the cache hierarchy */
  void touch(MemoryRequest request);

  /* Run a sequence of addresses through the cache hierarchy,
   * assuming the access doesn't cross cache-line boundaries */
  void touch(const std::vector<uint64_t>& addresses);

  /* Run a sequence of access through the cache hierarchy */
  void touch(const std::vector<SizedAccess>& accesses);

  /* Run a sequence of requests through the cache hierarchy */
  void touch(const std::vector<MemoryRequest>& requests);
};
