#include "catch.hpp"

#include <vector>

#include "utils.hh"

#include "CacheHierarchy.hh"

TEST_CASE("Hierarchies contain the correct number and type of caches", "[hierarchy]") {
  const auto nlevels = GENERATE(range(1, 3));
  const auto configs = std::vector<CacheConfig>(
      nlevels, get_default_cache_config(CacheType::SetAssociative));

  CacheHierarchy ch { configs };

  REQUIRE(ch.nlevels() == nlevels);

  for (int level = 1; level <= DEFAULT_HIERARCHY_SIZE; level++)
  {
    REQUIRE(ch.getType(1) == CacheType::SetAssociative);
    REQUIRE(ch.getSize(1) == DEFAULT_CACHE_SIZE);
    REQUIRE(ch.getLineSize(1) == DEFAULT_LINE_SIZE);
    REQUIRE(ch.getSetSize(1) == DEFAULT_SET_SIZE);
  }
}

TEST_CASE("Stats of caches in a hierarchy are initialised properly", "[hierarchy]") {
  const auto ch = make_default_hierarchy(CacheType::SetAssociative);

  for (int level = 1; level <= DEFAULT_HIERARCHY_SIZE; level++) {
    REQUIRE(ch->getHits(level) == 0);
    REQUIRE(ch->getMisses(level) == 0);
    REQUIRE(ch->getTotalAccesses(level) == 0);
    REQUIRE(ch->getEvictions(level) == 0);
  }
}
