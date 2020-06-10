#include "catch.hpp"

#include <fstream>
#include <sstream>

#include "utils.hh"

#include "TraceConverter.hh"

TEST_CASE("Converted traces are equal to originals", "[trace][converter-bin]") {
  const auto text_fname = try_tracefile_names("traces/8.trace");

  const std::string bin_fname { "8.bin" };
  const auto status = TraceConverter::convert(text_fname, bin_fname, true, false);
  REQUIRE(status == TraceConverter::ConvertStatus::Success);

  const MemoryTrace text_trace { std::ifstream { text_fname } };
  const MemoryTrace bin_trace { std::ifstream { bin_fname }, TraceFileType::Binary };
  REQUIRE(trace_equals(text_trace, bin_trace));
}

TEST_CASE("Converter does not overwrite output files without force", "[converter-bin]") {
  // Create a dummy file
  const std::string out_fname { "testout.bin" };
  std::ofstream f { out_fname };
  f << "Test\n";

  const auto status = TraceConverter::convert("test.in", out_fname, false, false);
  REQUIRE(status == TraceConverter::ConvertStatus::AlreadyExists);
}
