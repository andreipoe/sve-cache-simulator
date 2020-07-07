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

// TODO: This random generator is not seeded
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

MemoryRequest make_mem_request(uint64_t address, int size, bool is_write) {
  return MemoryRequest{0, size, 0, is_write, address, 0};
}

CacheConfig get_default_cache_config(CacheType type) {
  int set_size = type == CacheType::SetAssociative ? DEFAULT_SET_SIZE : 1;
  return { type, DEFAULT_CACHE_SIZE, DEFAULT_LINE_SIZE, set_size };
}

std::unique_ptr<Cache> make_default_cache(CacheType type) {
  return Cache::make_cache(get_default_cache_config(type), std::make_shared<Clock>());
}

std::unique_ptr<CacheHierarchy> make_default_hierarchy(CacheType type) {
  std::vector<CacheConfig> configs(DEFAULT_HIERARCHY_SIZE,
                                   get_default_cache_config(type));
  for (int level = DEFAULT_HIERARCHY_SIZE - 1; level >= 0; level--)
    configs[level].size /= (DEFAULT_HIERARCHY_SIZE - level);


  return std::make_unique<CacheHierarchy>(configs);
}

bool trace_equals(const MemoryTrace& trace1, const MemoryTrace& trace2) {
  if (trace1.getLength() != trace2.getLength()) return false;

  const auto requests1 = trace1.getRequests();
  const auto requests2 = trace2.getRequests();
  for (size_t i = 0; i < requests1.size(); i++) {
    const auto r1 = requests1[i];
    const auto r2 = requests2[i];

    if (r1.tid != r2.tid || r1.size != r2.size || r1.is_write != r2.is_write ||
        r1.bundle_kind != r2.bundle_kind || r1.address != r2.address || r1.pc != r2.pc)
      return false;
  }

  return true;
}
