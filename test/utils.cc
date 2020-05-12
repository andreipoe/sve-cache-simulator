#include "utils.hh"

#include <exception>
#include <filesystem>
#include <vector>

std::string try_tracefile_names(const std::string& name) {
  const std::vector<std::string> paths { "", "traces/", "test/", "test/traces/" };

  for (auto const& path : paths)
    if (std::filesystem::exists(path + name)) return path + name;

  throw std::invalid_argument("Cannot find trace file: " + name);
}

std::string try_configfile_names(const std::string& name) {
  const std::vector<std::string> paths { "", "configs/", "test/", "test/configs/",
                                         "../configs/" };

  for (auto const& path : paths)
    if (std::filesystem::exists(path + name)) return path + name;

  throw std::invalid_argument("Cannot find config file: " + name);
}
