#pragma once

#include <iostream>

enum class CacheType { Infinite, DirectMapped, SetAssociative };

struct CacheConfig {
  CacheType type;

  /* Total size in bytes */
  int size;

  /* Cache line size in bytes */
  int line_size;

  /* The size of a cache set, i.e. the "number of ways" */
  int set_size;

  CacheConfig(const CacheType type, const int size, const int line_size, const int set_size = 1);
  CacheConfig(std::istream&& config_file);
};
