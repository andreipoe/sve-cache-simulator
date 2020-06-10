#include <fstream>
#include <iostream>

#include "MemoryTrace.hh"
#include "TraceConverter.hh"

namespace TraceConverter {
namespace {
bool file_exists(const std::string& fname) {
  std::ifstream f { fname };
  return f.is_open();
}
}  // namespace

ConvertStatus convert(const std::string& in_fname, const std::string& out_fname,
                      bool force, bool display_progress) noexcept {
  if (display_progress) std::cout << in_fname << " --> " << out_fname << "... ";

  bool existed = file_exists(out_fname);
  if (existed && !force) {
    if (display_progress) std::cout << "EXISTS\n";
    return ConvertStatus::AlreadyExists;
  }

  try {
    MemoryTrace trace { std::ifstream { in_fname } };
    trace.write_binary(out_fname);

  } catch (std::exception& e) {
    if (display_progress) std::cout << "FAILED\n" << e.what() << "\n";
    return ConvertStatus::Error;
  }

  if (display_progress) {
    if (!existed)
      std::cout << "DONE\n";
    else
      std::cout << "OVERWRITTEN\n";
  }

  return ConvertStatus::Success;
}

std::string make_default_outname(const std::string& in_fname) {
  return in_fname.substr(0, in_fname.rfind('.')) + ".bin";
}

}  // namespace TraceConverter
