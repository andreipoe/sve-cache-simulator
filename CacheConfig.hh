#pragma once

#include <iostream>
#include <map>
#include <string>

using ConfigMap = std::map<std::string, std::string>;

enum class CacheType { Infinite, DirectMapped, SetAssociative };

struct CacheConfig {
  CacheType type;

  /* Total size in bytes */
  uint64_t size;

  /* Cache line size in bytes */
  int line_size;

  /* The size of a cache set, i.e. the "number of ways" */
  int set_size;

  CacheConfig(const CacheType type, const uint64_t size, const int line_size,
              const int set_size = 1);

  /* Construct a `CacheConfig` from a configuration file */
  CacheConfig(std::istream&& config_file);

  /* Construct a `CacheConfig` from a parameter mapping, as it would appear in a config
   * file */
  CacheConfig(ConfigMap config_map);

 private:
  void read_config_map_(ConfigMap config_map);
};
