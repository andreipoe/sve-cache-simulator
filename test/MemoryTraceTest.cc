#include "catch.hpp"

#include <fstream>
#include <set>

#include "utils.hh"

#include "MemoryTrace.hh"

TEST_CASE("Traces are loaded correctly") {
  auto filename = try_tracefile_names("traces/8.trace");

  std::ifstream tracefile(filename);
  MemoryTrace trace(tracefile);

  REQUIRE(trace.getLength() == 8);
  REQUIRE(trace.getRequestAddresses().size() == trace.getRequests().size());

  auto addresses = trace.getRequestAddresses();
  std::set<uint64_t> unique_addresses(addresses.begin(), addresses.end());
  REQUIRE(unique_addresses.size() == 7);
}
