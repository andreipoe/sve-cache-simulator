#pragma once

#include <iostream>

enum class CacheType { Infinite, DirectMapped };

struct CacheConfig {
  CacheType type;

  /* Total size in bytes */
  int size;

  /* Cache line size in bytes */
  int line_size;

  CacheConfig(CacheType type, int size, int line_size);
  CacheConfig(std::istream&& config_file);
};
