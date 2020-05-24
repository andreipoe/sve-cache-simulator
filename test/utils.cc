#include "utils.hh"

#include <exception>
#include <filesystem>
#include <random>
#include <set>
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

namespace random_address {
RandomAddressGenerator generator {};
}  // namespace random_address

// TODO: this should probably be seeded
uint64_t get_random_address() {
  using namespace random_address;
  generator.next();
  return generator.get();
}

std::vector<uint64_t> get_random_unique_addresses(int n, uint64_t except) {
  std::set<uint64_t> addresses { except };

  while (addresses.size() < static_cast<unsigned int>(n)) {
    uint64_t next;
    do {
      next = get_random_address();
    } while (addresses.find(next) != addresses.end());
    addresses.insert(next);
  }

  return std::vector(addresses.begin(), addresses.end());
}

CacheConfig get_default_cache_config(CacheType type) {
  int set_size = type == CacheType::SetAssociative ? DEFAULT_SET_SIZE : 1;
  return { type, DEFAULT_CACHE_SIZE, DEFAULT_LINE_SIZE, set_size };
}

std::unique_ptr<Cache> make_default_cache(CacheType type) {
  return Cache::make_cache(get_default_cache_config(type));
}
