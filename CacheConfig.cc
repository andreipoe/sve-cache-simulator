#include "CacheConfig.hh"

#include <string>

#include "inipp.h"

CacheConfig::CacheConfig(const CacheType type, const int size, const int line_size,
                         const int set_size)
    : type(type), size(size), line_size(line_size), set_size(set_size) {}

CacheConfig::CacheConfig(std::istream&& config_file) {
  inipp::Ini<char> ini;
  ini.parse(config_file);

  // Check that at most one level is defined
  // Otherwise, a CacheHierarcy, not a single Cache, should be build from this file
  if (ini.sections.size() == 1) {
    if (!ini.sections.begin()->first.empty())
      throw std::invalid_argument("Invalid section in cache config file: " +
                                  ini.sections.begin()->first);
  } else if (ini.sections.size() > 2)
    throw std::invalid_argument("Too many sections section in cache config file: " +
                                std::to_string(ini.sections.size()));
  else {
    // If there are exactly two sections, they must be "hierarchy" and "level1"
    for (const auto& section : ini.sections)
      if (section.first != "hierarchy" && section.first != "level1")
        throw std::invalid_argument("Invalid section in cache config file: " +
                                    section.first);

    try {
      const int nlevels = std::stoi(ini.sections.at("hierarchy").at("nlevels"));
      if (nlevels != 1)
        throw std::invalid_argument("Too many levels for a single cache: " +
                                    std::to_string(nlevels));
    } catch (const std::out_of_range& e) {
      std::cout << "Malformed config file: " << e.what() << "\n";
    }
  }


  auto config_section =
      ini.sections.size() == 1 ? ini.sections.at("") : ini.sections.at("level1");
  read_config_map_(config_section);
}

CacheConfig::CacheConfig(ConfigMap config_map) { read_config_map_(config_map); }


void CacheConfig::read_config_map_(ConfigMap config_map) {
  try {
    size      = std::stoi(config_map["cache_size"]);
    line_size = std::stoi(config_map["line_size"]);
    set_size  = config_map.find("set_size") != std::end(config_map)
                   ? std::stoi(config_map["set_size"])
                   : 1;
  } catch (const std::out_of_range& e) {
    throw std::invalid_argument(std::string("Malformed config file: ") + e.what());
  }

  auto typestr = config_map["type"];
  typestr.erase(std::remove_if(typestr.begin(), typestr.end(), [](unsigned char c) {
    return std::isspace(c) || std::ispunct(c);
  }));
  std::transform(typestr.begin(), typestr.end(), typestr.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (typestr == "infinite")
    type = CacheType::Infinite;
  else if (typestr == "directmapped")
    type = CacheType::DirectMapped;
  else if (typestr == "setassociative")
    type = CacheType::SetAssociative;
  else {
    throw std::invalid_argument("Invalid cache type in config file: " + typestr);
  }
}
