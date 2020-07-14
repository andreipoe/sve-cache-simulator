#include "CacheHierarchy.hh"

#include "inipp.h"

#define SECTION_HIERARCHY "hierarchy"
#define SECTION_LEVEL     "L"

#define KEY_NLEVELS "levels"


void CacheHierarchy::constuctor_common_() {
  if (std::any_of(std::begin(levels), std::end(levels),
                  [&](const std::unique_ptr<Cache>& c) {
                    return c->getLineSize() != levels[0]->getLineSize();
                  }))
    throw std::invalid_argument(
        "Cache hierarchy does not have the same line size throughout");
}

CacheHierarchy::CacheHierarchy(const std::vector<CacheConfig>& cache_configs)
    : traffic(cache_configs.size() + 1, 0), clock_(std::make_shared<Clock>()) {
  levels.reserve(cache_configs.size());
  for (const auto& config : cache_configs)
    levels.push_back(Cache::make_cache(config, clock_));

  constuctor_common_();
}

CacheHierarchy::CacheHierarchy(std::istream&& config_file)
    : clock_(std::make_shared<Clock>()) {
  inipp::Ini<char> ini;
  ini.parse(config_file);

  if (ini.sections.find(SECTION_HIERARCHY) == ini.sections.end())
    throw std::invalid_argument("Cache hierarchy depth not defined");

  const auto hierarchy_section = ini.sections.at(SECTION_HIERARCHY);
  int nlevels;
  try {
    nlevels = std::stoi(hierarchy_section.at(KEY_NLEVELS));
  } catch (const std::out_of_range& e) {
    throw std::invalid_argument(std::string("Malformed config file: ") + e.what());
  }

  traffic = std::vector<uint64_t>(nlevels + 1, 0);
  levels.reserve(nlevels);

  for (int level = 1; level <= nlevels; level++) {
    const auto this_level_header = SECTION_LEVEL + std::to_string(level);

    if (ini.sections.find(this_level_header) == ini.sections.end()) {
      std::ostringstream ss;
      ss << "Cache Hierarchy of " << nlevels << " does not define level " << level;
      throw std::invalid_argument(ss.str());
    }

    const auto this_level_section = ini.sections.at(this_level_header);
    levels.push_back(Cache::make_cache({ this_level_section }, clock_));
  }

  constuctor_common_();
}

// ------

uint64_t CacheHierarchy::current_cycle() const { return clock_->current_cycle(); }

// ------

int CacheHierarchy::nlevels() const { return levels.size(); }

CacheType CacheHierarchy::getType(int level) const {
  return levels[level - 1]->getType();
}

int CacheHierarchy::getSize(int level) const { return levels[level - 1]->getSize(); }

int CacheHierarchy::getLineSize(int level) const {
  return levels[level - 1]->getLineSize();
}
int CacheHierarchy::getSetSize(int level) const {
  return levels[level - 1]->getSetSize();
}

uint64_t CacheHierarchy::getHits(int level) const { return levels[level - 1]->getHits(); }

uint64_t CacheHierarchy::getMisses(int level) const {
  return levels[level - 1]->getMisses();
}
uint64_t CacheHierarchy::getTotalAccesses(int level) const {
  return levels[level - 1]->getHits() + levels[level - 1]->getMisses();
}
uint64_t CacheHierarchy::getEvictions(int level) const {
  return levels[level - 1]->getEvictions();
}

uint64_t CacheHierarchy::getTraffic(int from_level) const { return traffic[from_level]; }

const std::map<uint64_t, BundleStats>& CacheHierarchy::getBundleOps() const { return bundles; }

std::unique_ptr<std::map<uint64_t, uint64_t>> CacheHierarchy::getLifetimes(
    int level) const {
  return levels[level - 1]->getLifetimes();
}

// ------

void CacheHierarchy::touch(uint64_t address, int size,
                           __attribute__((unused)) bool is_write) {
  const int line_size = levels[0]->getLineSize();

  traffic[0] += size;

  uint64_t next_address { address };
  int remaining_size { size };

  while (remaining_size > 0) {

    // Go through cache levels in reverse order until either all accesses are hits or
    // we've reached the top level
    for (int current_level = 0; current_level < nlevels(); current_level++) {
      // Find the first cache line this request touches
      const auto& cache_address = levels[current_level]->split_address(next_address);
      const auto events         = levels[current_level]->touch(cache_address);

      // If counting writebacks as accesses, only break if the access is not a write
      // But doing so seems to hugely increase the number of accesses, unlike what
      // hardware counters report
      if (events.hit()) break;

      traffic[current_level + 1] += line_size;
    }

    // Skip over the remaining bytes in this same cache line
    const unsigned int covered_bytes =
        line_size - levels[0]->split_address(next_address).block;
    remaining_size -= covered_bytes;
    next_address += covered_bytes;
  }

  clock_->tick();
}

void CacheHierarchy::touch(MemoryRequest request) {
  if (request.is_bundle()) {
    bundles[request.pc].total_ops++;
    if (request.is_bundle_start()) bundles[request.pc].times_encountered++;
  }
  touch(request.address, request.size, request.is_write);
}

void CacheHierarchy::touch(const std::vector<MemoryRequest>& requests) {
  for (auto const& r : requests) touch(r);
}
