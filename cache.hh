#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "CacheConfig.hh"

enum class CacheEvent { Hit, Miss };

struct CacheAddress {
  long tag;
  int index, block;
};

struct CacheEntry {
  long tag;
  bool valid = false;
};

class NotImplementedException : public std::logic_error {
 public:
  NotImplementedException();
};

class Cache {
 protected:
  int size, line_size;

  long hits = 0, misses = 0;

  explicit Cache(int size, int line_size);
  Cache(CacheConfig config);

  virtual const CacheAddress split_address(const long address) const;

 public:
  virtual ~Cache();

  /* Run a single address through the cache */
  virtual const CacheEvent touch(const long address) = 0;

  /* Run a sequence of addresses through the cache */
  void touch(const std::vector<long> addresses);

  const int getSize() const;
  const int getLineSize() const;

  const long getHits() const;
  const long getMisses() const;
  const long getTotalAccesses() const;

  static std::unique_ptr<Cache> make_cache(const CacheConfig config);
};
