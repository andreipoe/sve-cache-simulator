#include "CacheConfig.hh"

#include <string>

#include "inipp.h"

CacheConfig::CacheConfig(const CacheType type, const int size, const int line_size,
                         const int set_size)
    : type(type), size(size), line_size(line_size), set_size(set_size) {}

CacheConfig::CacheConfig(std::istream&& config_file) {
  inipp::Ini<char> ini;
  ini.parse(config_file);

  // TODO: use sections for levels
  auto default_section = ini.sections[""];

  try {
    size      = std::stoi(default_section["cache_size"]);
    line_size = std::stoi(default_section["line_size"]);
    set_size  = default_section.find("set_size") != std::end(default_section)
                   ? std::stoi(default_section["set_size"])
                   : 1;
  } catch (const std::out_of_range& e) {
    std::cout << "Malformed config file: " << e.what() << "\n";
    std::exit(2);
  }

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
  else if (typestr == "setassociative")
    type = CacheType::SetAssociative;
  else {
    std::cout << "Invalid cache type ing config file: " << typestr << "\n";
    exit(1);
  }
}
