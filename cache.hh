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
  int size, line_size;

  uint64_t hits { 0 }, misses { 0 };

  explicit Cache(const int size, const int line_size);
  Cache(const CacheConfig config);

  virtual const CacheAddress split_address(const uint64_t address) const;

 public:
  virtual ~Cache();

  /* Run a single address through the cache */
  virtual  CacheEvent touch(const uint64_t address) = 0;

  /* Run a sequence of addresses through the cache */
  void touch(const std::vector<long> addresses);

  int getSize() const;
  int getLineSize() const;

  uint64_t getHits() const;
  uint64_t getMisses() const;
  uint64_t getTotalAccesses() const;

  static std::unique_ptr<Cache> make_cache(const CacheConfig config);
};
