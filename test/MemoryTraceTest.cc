#include "catch.hpp"

#include <fstream>
#include <set>
#include <sstream>

#include "utils.hh"

#include "MemoryTrace.hh"

TEST_CASE("Trace files are loaded correctly", "[trace]") {
  auto filename = try_tracefile_names("traces/8.trace");

  std::ifstream tracefile(filename);
  MemoryTrace trace(tracefile);

  REQUIRE(trace.getLength() == 8);
  REQUIRE(trace.getRequestAddresses().size() == trace.getRequests().size());

  auto addresses = trace.getRequestAddresses();
  std::set<uint64_t> unique_addresses(addresses.begin(), addresses.end());
  REQUIRE(unique_addresses.size() == 7);
}

TEST_CASE("Memory requests are constructed correctly from trace entries", "[trace]") {
  const int seq { 42 }, tid { 12 };
  const int size        = GENERATE(1, 8, 16, 32, 64, 128, 256, 512, 1024, 2048);
  const int bundle_kind = GENERATE(0, 1, 2, 3, 4, 6, 7);
  const bool is_write   = GENERATE(0, 1);
  const uint64_t address { get_random_address() }, pc { get_random_address() };

  // Make a string that looks like a memory trace entry
  std::stringstream ss;
  const auto sep = ", ";
  ss << seq << sep << tid << sep << bundle_kind << sep << is_write << sep << size << sep;
  ss << std::hex << "0x" << address << sep << "0x" << pc << "\n";

  // Parse it
  const MemoryTrace trace { ss };
  const MemoryRequest req = trace.getRequests()[0];

  // Check the the produced request has the same values as the input
  REQUIRE(req.size == size);
  REQUIRE(req.tid == tid);
  REQUIRE(req.bundle_kind == bundle_kind);
  REQUIRE(req.is_write == is_write);
  REQUIRE(req.address == address);
  REQUIRE(req.pc == pc);
}

TEST_CASE("Empty lines in trace files are skipped over", "[trace][regression]") {
  std::istringstream ss {
    "\n"
    "214864667, 0, 0, 0, 64, 0x4000847ad870, 0x434edc\n\n"
    "214864668, 0, 0, 0, 64, 0x4000863fb8f0, 0x434ef4\n"
  };
  const MemoryTrace trace { ss };

  REQUIRE(trace.getLength() == 2);

  const auto addresses = trace.getRequestAddresses();
  REQUIRE(addresses[0] == 0x4000847ad870);
  REQUIRE(addresses[1] == 0x4000863fb8f0);
}

TEST_CASE("Writing and parsing binary trace files works", "[trace]") {
  std::istringstream ss {
    "4016124, 0, 0, 1, 64, 0x6e0000, 0x40e370\n"
    "4016126, 0, 0, 0, 64, 0x630a00, 0x40e360\n"
  };
  const MemoryTrace trace { ss };

  const std::string fname { "testout.bin" };
  trace.write_binary(fname);

  const MemoryTrace binary_trace { std::ifstream { fname }, TraceFileType::Binary };
  REQUIRE(trace_equals(trace, binary_trace));
}
