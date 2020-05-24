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

  for (int level = 1; level <= DEFAULT_HIERARCHY_SIZE; level++) {
    REQUIRE(ch.getType(1) == CacheType::SetAssociative);
    REQUIRE(ch.getSize(1) == DEFAULT_CACHE_SIZE);
    REQUIRE(ch.getLineSize(1) == DEFAULT_LINE_SIZE);
    REQUIRE(ch.getSetSize(1) == DEFAULT_SET_SIZE);
  }
}

TEST_CASE("Hierarchies with varying line sizes are not allowed", "[hierarchy]") {
  std::vector<CacheConfig> levels(2, get_default_cache_config(CacheType::SetAssociative));
  levels[1].size      = DEFAULT_CACHE_SIZE / 4;
  levels[1].line_size = DEFAULT_LINE_SIZE / 2;

  REQUIRE_THROWS_WITH(CacheHierarchy { levels },
                      "Cache hierarchy does not have the same line size throughout");
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

TEST_CASE("First hierarchy touch always misses all levels", "[hierarchy]") {
  const auto ch = make_default_hierarchy(CacheType::SetAssociative);

  const uint64_t address = GENERATE(take(DEFAULT_RANDOM_COUNT, random_addresses()));

  ch->touch(address);
  for (int level = 1; level <= DEFAULT_HIERARCHY_SIZE; level++) {
    REQUIRE(ch->getHits(level) == 0);
    REQUIRE(ch->getMisses(level) == 1);
  }
}

TEST_CASE("Second hierarchy touch always hits all levels", "[hierarchy]") {
  const auto ch = make_default_hierarchy(CacheType::SetAssociative);

  const uint64_t address = GENERATE(take(DEFAULT_RANDOM_COUNT, random_addresses()));

  ch->touch(address);  // all miss
  ch->touch(address);  // hit on LLC

  for (int level = 1; level <= DEFAULT_HIERARCHY_SIZE; level++)
    REQUIRE(ch->getMisses(level) == 1);

  REQUIRE(ch->getHits(ch->nlevels()) == 1);
  for (int level = 1; level <= DEFAULT_HIERARCHY_SIZE - 1; level++)
    REQUIRE(ch->getHits(level) == 0);
}

// TODO: tests accesses that only miss some levels
