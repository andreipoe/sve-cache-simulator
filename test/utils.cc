#include "utils.hh"

#include <exception>
#include <filesystem>
#include <vector>

std::string try_tracefile_names(std::string name) {
  std::vector<std::string> paths{"", "traces/", "test/", "test/traces/"};

  for (auto const& path : paths)
    if (std::filesystem::exists(path + name)) return path + name;

  throw std::invalid_argument("Cannot find trace file: " + name);
}
