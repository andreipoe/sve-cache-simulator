#pragma once

#include <stdexcept>

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

  virtual const CacheEvent touch(long address) = 0;

  const long getHits() const;
  const long getMisses() const;
  const long getTotalAccesses() const;
};
