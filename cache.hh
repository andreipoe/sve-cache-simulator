#pragma once

enum class CacheEvent { Hit, Miss };

class Cache {
 protected:
  long hits = 0, misses = 0;

  Cache();

 public:
  virtual ~Cache();

  virtual const CacheEvent touch(long address) = 0;

  const long getHits() const;
  const long getMisses() const;
  const long getTotalAccesses() const;
};
