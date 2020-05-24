#include "CacheHierarchy.hh"

#include "inipp.h"

#define SECTION_HIERARCHY "hierarchy"
#define SECTION_LEVEL     "level"

#define KEY_NLEVELS "levels"

CacheHierarchy::CacheHierarchy(std::vector<CacheConfig> cache_configs) {
  levels.reserve(cache_configs.size());
  for (const auto& config : cache_configs) levels.push_back(Cache::make_cache(config));
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
}

int CacheHierarchy::nlevels() const { return levels.size(); }
int CacheHierarchy::getSize(int level) const { return levels[level - 1]->getSize(); }
int CacheHierarchy::getLineSize(int level) const {
  return levels[level - 1]->getLineSize();
}
int CacheHierarchy::getSetSize(int level) const {
  return levels[level - 1]->getSetSize();
}
