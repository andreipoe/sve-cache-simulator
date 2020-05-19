#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "CacheConfig.hh"

enum class CacheEvent { Hit, Miss };

class Cache;

/* A memory address split into the cache indexing components */
struct CacheAddress {
  uint64_t tag;
  unsigned int index, block;

  explicit CacheAddress(uint64_t address, uint64_t cache_size, unsigned int line_size);
  explicit CacheAddress(uint64_t address, const CacheConfig& config);
  explicit CacheAddress(uint64_t address, const Cache& cache);
};

/* A cache row entry (without the value, because it's never needed) */
struct CacheEntry {
  uint64_t tag;
  bool valid { false };
};

class NotImplementedException : public std::logic_error {
 public:
  NotImplementedException();
};

class Cache {
 protected:
  /* The total size of the cache, in bytes */
  const uint64_t size;

  /* The size of a cache line, in bytes */
  const int line_size;

  /* The numer of bits required to address a cache line */
  const int block_bits;

  /* The numer of bits required to address a cache set */
  const int index_bits;

  uint64_t hits { 0 }, misses { 0 };

  explicit Cache(const uint64_t size, const int line_size);
  Cache(const CacheConfig config);

 public:
  virtual ~Cache();

  /* Run a single address through the cache,
   * assuming the access doesn't cross cache-line boundaries */
  virtual CacheEvent touch(const CacheAddress& address) = 0;

  // TODO: return the cache events
  /* Run a single request through the cache */
  virtual void touch(const uint64_t address, const int size = 1) final;

  // TODO: return the cache events
  /* Run a sequence of addresses through the cache,
   * assuming the access doesn't cross cache-line boundaries */
  virtual void touch(const std::vector<uint64_t> addresses) final;

  int getSize() const;
  int getLineSize() const;

  uint64_t getHits() const;
  uint64_t getMisses() const;
  uint64_t getTotalAccesses() const;

  static std::unique_ptr<Cache> make_cache(const CacheConfig config);

  virtual const CacheAddress split_address(const uint64_t address) const final;
};
