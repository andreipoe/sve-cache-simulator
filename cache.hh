#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "CacheConfig.hh"
#include "Clock.hh"
#include "MemoryTrace.hh"

struct CacheEvents {
  uint64_t hits { 0 }, misses { 0 }, evictions { 0 };

  /* Returns true if all events counted are hits */
  bool hit() const;

  friend CacheEvents operator+(CacheEvents lhs, const CacheEvents& rhs) {
    lhs.hits += rhs.hits;
    lhs.misses += rhs.misses;
    lhs.evictions += rhs.evictions;

    return lhs;
  }

  CacheEvents& operator+=(const CacheEvents& rhs);
};

/* A tuple holding a memory address to be accessed and the size of the access */
struct SizedAccess {
  uint64_t address;
  int size;
};

class Cache;

/* A memory address split into the cache indexing components */
struct CacheAddress {
  uint64_t tag;
  unsigned int index, block;

  explicit CacheAddress(uint64_t address, uint64_t cache_size, int line_size,
                        int set_size);
  explicit CacheAddress(uint64_t address, const CacheConfig& config);
  explicit CacheAddress(uint64_t address, const Cache& cache);

  friend std::ostream& operator<<(std::ostream& stream,
                                  const CacheAddress& cache_address) {
    stream << "Address{ tag: " << cache_address.tag << ", index: " << cache_address.index
           << ", block: " << cache_address.block << " }";
    return stream;
  }
};

/* A cache row entry (without the value, because it's never needed) */
struct CacheEntry {
  uint64_t tag;

  /* Shows where this entry has ever been touched */
  bool valid { false };

  /* How long this entry has been in the cache for. Incremented on every touch to this
   * entry's set */
  uint64_t age { 0 };

  /* The cycle on which this entry was loaded into the cache. Only makes sense if the
   * entry is valid. */
  uint64_t loaded_at;

  CacheEntry()                     = default;
  CacheEntry(const CacheEntry& ce) = default;
  CacheEntry(CacheEntry&& ce)      = default;
  CacheEntry& operator=(const CacheEntry& ce) = default;
  CacheEntry& operator=(CacheEntry&& ce) = default;

  /* Marks that data has been loaded into this cache entry, making it valid and recording
   * the load timestamp */
  void set(uint64_t tag, uint64_t timestamp);
};

class NotImplementedException : public std::logic_error {
 public:
  NotImplementedException();
};

/* An exception signaling a problem with a configuration file */
class ConfigException : public std::logic_error {
 public:
  ConfigException();
};

// ------

class Cache {
 protected:
  /* The total size of the cache, in bytes */
  const uint64_t size;

  /* The size of a cache line, in bytes */
  const int line_size;

  /* The size of a cache set, i.e. the "number of ways" */
  const int set_size;

  uint64_t hits { 0 }, misses { 0 }, evictions { 0 };

  /* The hierarchy's clock, shared between all the levels */
  const std::shared_ptr<const Clock> clock_;

  /* A histogram of how long cache lines last in this cache before being evicted */
  std::map<uint64_t, uint64_t> lifetimes;


  explicit Cache(const uint64_t size, const int line_size, const int set_size,
                 const std::shared_ptr<const Clock> clock);
  Cache(const CacheConfig& config, const std::shared_ptr<const Clock> clock);


  /* Add an entry to the lifetimes histogram for a line loaded on the given cycle and
   * evicted on this cycle */
  void log_eviction(uint64_t loaded_at);

 public:
  virtual ~Cache();

  /* Run a single address through the cache,
   * assuming the access doesn't cross cache-line boundaries */
  virtual CacheEvents touch(const CacheAddress& address) = 0;

  /* Run a single request through the cache */
  virtual CacheEvents touch(const uint64_t address, const int size = 1) final;

  /* Run a single request through the cache */
  virtual CacheEvents touch(const SizedAccess& access) final;

  /* Run a single request through the cache */
  virtual CacheEvents touch(const MemoryRequest& request) final;

  /* Run a sequence of addresses through the cache,
   * assuming the access doesn't cross cache-line boundaries */
  virtual CacheEvents touch(const std::vector<uint64_t>& addresses) final;

  /* Run a sequence of addresses that don't cross cache-line boundaries through the cache
   */
  virtual CacheEvents touch(const std::vector<CacheAddress>& addresses) final;

  /* Run a sequence of accesses through the cache */
  virtual CacheEvents touch(const std::vector<SizedAccess>& accesses) final;

  /* Run a sequence of requests through the cache */
  virtual CacheEvents touch(const std::vector<MemoryRequest>& requests) final;


  virtual uint64_t getSize() const final;
  virtual int getLineSize() const final;
  virtual int getSetSize() const final;
  virtual CacheType getType() const = 0;

  uint64_t getHits() const;
  uint64_t getMisses() const;
  uint64_t getTotalAccesses() const;
  uint64_t getEvictions() const;
  const std::map<uint64_t, uint64_t>& getLifetimes() const;


  /* Factory method for creating caches based on the given configuration */
  static std::unique_ptr<Cache> make_cache(const CacheConfig& config,
                                           const std::shared_ptr<const Clock> clock);

  /* Split a raw address into a tag, a set, a line, and a block, as mapped by this cache
   */
  virtual const CacheAddress split_address(const uint64_t address) const final;
};
