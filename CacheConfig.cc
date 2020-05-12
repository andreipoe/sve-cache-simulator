#include "CacheConfig.hh"

#include <string>

#include "inipp.h"

CacheConfig::CacheConfig(CacheType type, int size, int line_size)
    : type(type), size(size), line_size(line_size) {}

CacheConfig::CacheConfig(std::istream&& config_file) {
  inipp::Ini<char> ini;
  ini.parse(config_file);

  // TODO: use sections
  auto default_section = ini.sections[""];

  size = std::stoi(default_section["cache_size"]);
  line_size = std::stoi(default_section["line_size"]);

  auto typestr = default_section["type"];
  typestr.erase(std::remove_if(typestr.begin(), typestr.end(), [](unsigned char c) {
    return std::isspace(c) || std::ispunct(c);
  }));
  std::transform(typestr.begin(), typestr.end(), typestr.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (typestr == "infinite")
    type = CacheType::Infinite;
  else if (typestr == "directmapped")
    type = CacheType::DirectMapped;
}
