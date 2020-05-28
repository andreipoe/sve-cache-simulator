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
  const int size       = GENERATE(1, 8, 16, 32, 64, 128, 256, 512, 1024, 2048);
  const bool is_bundle = GENERATE(0, 1), is_write = GENERATE(0, 1);
  const uint64_t address { get_random_address() }, pc { get_random_address() };

  // Make a string that looks like a memory trace entry
  std::stringstream ss;
  const auto sep = ", ";
  ss << seq << sep << tid << sep << is_bundle << sep << is_write << sep << size << sep;
  ss << std::hex << "0x" << address << sep << "0x" << pc << "\n";

  // Parse it
  const MemoryTrace trace { ss };
  const MemoryRequest req = trace.getRequests()[0];

  // Check the the produced request has the same values as the input
  REQUIRE(req.size == size);
  REQUIRE(req.tid == tid);
  REQUIRE(req.is_bundle == is_bundle);
  REQUIRE(req.is_write == is_write);
  REQUIRE(req.address == address);
  REQUIRE(req.pc == pc);
}
