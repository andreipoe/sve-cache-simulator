#include "catch.hpp"

#include <vector>

#include "utils.hh"

#include "CacheHierarchy.hh"

TEST_CASE("Hierarchies contained the correct number of caches", "[hierarchy]") {
  const auto nlevels = GENERATE(range(1, 3));
  const auto configs = std::vector<CacheConfig>(
      nlevels, get_default_cache_config(CacheType::DirectMapped));

  CacheHierarchy ch { configs };

  REQUIRE(ch.nlevels() == nlevels);
}
