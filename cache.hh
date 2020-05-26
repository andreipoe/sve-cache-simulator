#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "CacheConfig.hh"

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
  bool valid { false };
  uint64_t age { 0 };

  CacheEntry()                     = default;
  CacheEntry(const CacheEntry& mE) = default;
  CacheEntry(CacheEntry&& mE)      = default;
  CacheEntry& operator=(const CacheEntry& mE) = default;
  CacheEntry& operator=(CacheEntry&& mE) = default;

  CacheEntry(uint64_t tag);
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

class Cache {
 protected:
  /* The total size of the cache, in bytes */
  const uint64_t size;

  /* The size of a cache line, in bytes */
  const int line_size;

  /* The size of a cache set, i.e. the "number of ways" */
  const int set_size;

  uint64_t hits { 0 }, misses { 0 }, evictions { 0 };

  explicit Cache(const uint64_t size, const int line_size, const int set_size = 1);
  Cache(const CacheConfig config);

 public:
  virtual ~Cache();

  /* Run a single address through the cache,
   * assuming the access doesn't cross cache-line boundaries */
  virtual CacheEvents touch(const CacheAddress& address) = 0;

  /* Run a single request through the cache */
  virtual CacheEvents touch(const uint64_t address, const int size = 1) final;

  /* Run a single request through the cache */
  virtual CacheEvents touch(const SizedAccess access) final;

  /* Run a sequence of addresses through the cache,
   * assuming the access doesn't cross cache-line boundaries */
  virtual CacheEvents touch(const std::vector<uint64_t> addresses) final;

  /* Run a sequence of addresses that don't cross cache-line boundaries through the cache
   */
  virtual CacheEvents touch(const std::vector<CacheAddress> addresses) final;

  /* Run a sequence of acceesses through the cache */
  virtual CacheEvents touch(const std::vector<SizedAccess> accesses) final;

  virtual uint64_t getSize() const final;
  virtual int getLineSize() const final;
  virtual int getSetSize() const final;
  virtual CacheType getType() const = 0;

  uint64_t getHits() const;
  uint64_t getMisses() const;
  uint64_t getTotalAccesses() const;
  uint64_t getEvictions() const;

  /* Factory method for creating caches based on the given configuration */
  static std::unique_ptr<Cache> make_cache(const CacheConfig config);

  /* Split a raw address into a tag, a set, a line, and a block, as mapped by this cache
   */
  virtual const CacheAddress split_address(const uint64_t address) const final;
};
