#include "utils.hh"

#include <exception>
#include <filesystem>
#include <vector>

std::string try_tracefile_names(const std::string& name) {
  const std::vector<std::string> paths { "",         "traces/",
                                         "test/",    "test/traces/",
                                         "../",      "../traces/",
                                         "../test/", "../test/traces/" };

  for (auto const& path : paths)
    if (std::filesystem::exists(path + name)) return path + name;

  throw std::invalid_argument("Cannot find trace file: " + name);
}

std::string try_configfile_names(const std::string& name) {
  const std::vector<std::string> paths { "",         "configs/",
                                         "test/",    "test/configs/",
                                         "../",      "../configs/",
                                         "../test/", "../test/configs/" };

  for (auto const& path : paths)
    if (std::filesystem::exists(path + name)) return path + name;

  throw std::invalid_argument("Cannot find config file: " + name);
}

std::unique_ptr<Cache> make_default_cache(CacheType type) {
  return Cache::make_cache({ type, DEFAULT_CACHE_SIZE, DEFAULT_LINE_SIZE });
}
