#pragma once

#include <stdexcept>
#include <vector>

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
  long hits = 0, misses = 0;

  Cache();

  virtual const CacheAddress split_address(long address) const;

 public:
  virtual ~Cache();

  /* Run a single address through the cache */
  virtual const CacheEvent touch(long address) = 0;

  /* Run a sequence of addresses through the cache */
  virtual void touch(std::vector<long> addresses);

  const long getHits() const;
  const long getMisses() const;
  const long getTotalAccesses() const;
};
