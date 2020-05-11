#include "catch.hpp"

#include <fstream>
#include <set>

#include "MemoryTrace.hh"
#include "utils.hh"

TEST_CASE("Traces are loaded correctly") {
  auto filename = try_tracefile_names("traces/8.trace");

  std::ifstream tracefile(filename);
  MemoryTrace trace(tracefile);

  REQUIRE(trace.getLength() == 8);
  REQUIRE(trace.getRequestAddresses().size() == trace.getRequests().size());

  auto addresses = trace.getRequestAddresses();
  std::set<long> unique_addresses(addresses.begin(), addresses.end());
  REQUIRE(unique_addresses.size() == 7);
}
