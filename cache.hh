#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "CacheConfig.hh"

enum class CacheEvent { Hit, Miss };

struct CacheAddress {
  uint64_t tag;
  int index, block;
};

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

  virtual const CacheAddress split_address(const uint64_t address) const final;

 public:
  virtual ~Cache();

  /* Run a single address through the cache */
  virtual CacheEvent touch(const uint64_t address) = 0;

  /* Run a sequence of addresses through the cache */
  void touch(const std::vector<uint64_t> addresses);

  int getSize() const;
  int getLineSize() const;

  uint64_t getHits() const;
  uint64_t getMisses() const;
  uint64_t getTotalAccesses() const;

  static std::unique_ptr<Cache> make_cache(const CacheConfig config);
};
