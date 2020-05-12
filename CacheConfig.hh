#pragma once

#include <iostream>

enum class CacheType { Infinite, DirectMapped };

struct CacheConfig {
  CacheType type;

  /* Total size in bytes */
  int size;

  /* Cache line size in bytes */
  int line_size;

  CacheConfig(const CacheType type, const int size, const int line_size);
  CacheConfig(std::istream&& config_file);
};
