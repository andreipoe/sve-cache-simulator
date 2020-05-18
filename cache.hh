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

  explicit Cache(const int size, const int line_size);
  Cache(const CacheConfig config);

  virtual const CacheAddress split_address(const long address) const;

 public:
  virtual ~Cache();

  /* Run a single address through the cache */
  virtual  CacheEvent touch(const long address) = 0;

  /* Run a sequence of addresses through the cache */
  void touch(const std::vector<long> addresses);

  int getSize() const;
  int getLineSize() const;

  long getHits() const;
  long getMisses() const;
  long getTotalAccesses() const;

  static std::unique_ptr<Cache> make_cache(const CacheConfig config);
};
