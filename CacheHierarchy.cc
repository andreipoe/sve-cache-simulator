#include "CacheHierarchy.hh"

#include "inipp.h"

#define SECTION_HIERARCHY "hierarchy"
#define SECTION_LEVEL     "level"

#define KEY_NLEVELS "levels"

CacheHierarchy::CacheHierarchy(const std::vector<CacheConfig> cache_configs) {
  levels.reserve(cache_configs.size());
  for (const auto& config : cache_configs) levels.push_back(Cache::make_cache(config));

  if (std::any_of(std::begin(levels), std::end(levels),
                  [&](const std::unique_ptr<Cache>& c) {
                    return c->getLineSize() != levels[0]->getLineSize();
                  }))
    throw std::invalid_argument(
        "Cache hierarchy does not have the same line size throughout");
}

CacheHierarchy::CacheHierarchy(std::istream&& config_file) {
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
  levels.reserve(nlevels);

  for (int level = 1; level <= nlevels; level++) {
    const auto this_level_header = SECTION_LEVEL + std::to_string(level);

    if (ini.sections.find(this_level_header) == ini.sections.end()) {
      std::ostringstream ss;
      ss << "Cache Hierarchy of " << nlevels << " does not define level " << level;
      throw std::invalid_argument(ss.str());
    }

    const auto this_level_section = ini.sections.at(this_level_header);
    levels.push_back(Cache::make_cache({ this_level_section }));
  }

  if (std::any_of(std::begin(levels), std::end(levels),
                  [&](const std::unique_ptr<Cache>& c) {
                    return c->getLineSize() != levels[0]->getLineSize();
                  }))
    throw std::invalid_argument(
        "Cache hierarchy does not have the same line size throughout");
}

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


void CacheHierarchy::touch(uint64_t address, int size) {
  const int line_size = levels[0]->getLineSize();

  uint64_t next_address { address };
  int remaining_size { size };

  while (remaining_size > 0) {
    CacheEvents events;
    int current_level { 0 };

    // Go through cache levels in reverse order until either all accesses are hits or
    // we've reached the top level
    do {
      // Find the first cache line this request touches
      auto const& cache_address = levels[current_level]->split_address(next_address);
      events                    = levels[current_level]->touch(cache_address);
      current_level++;
    } while (!events.hit() && current_level < nlevels());

    // Skip over the remaining bytes in this same cache line
    const unsigned int covered_bytes =
        line_size - levels[0]->split_address(next_address).block;
    remaining_size -= covered_bytes;
    next_address += covered_bytes;
  }
}

void CacheHierarchy::touch(const std::vector<uint64_t> addresses) {
  for (auto const& a : addresses) touch(a);
}
