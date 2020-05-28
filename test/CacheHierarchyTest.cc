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

TEST_CASE("Second hierarchy touch always hits first level", "[hierarchy]") {
  const auto ch = make_default_hierarchy(CacheType::SetAssociative);

  const uint64_t address = GENERATE(take(DEFAULT_RANDOM_COUNT, random_addresses()));

  ch->touch(address);  // all miss
  ch->touch(address);  // hit on L1

  for (int level = 1; level <= DEFAULT_HIERARCHY_SIZE; level++)
    REQUIRE(ch->getMisses(level) == 1);

  REQUIRE(ch->getHits(1) == 1);
  for (int level = 2; level <= DEFAULT_HIERARCHY_SIZE; level++)
    REQUIRE(ch->getHits(level) == 0);
}

TEST_CASE("Levels in a hierarchy hit and miss as expected", "[hierarchy]") {
  const int hierarchy_size { 3 };
  const int size_per_level { 1024 };

  std::vector<CacheConfig> configs(hierarchy_size,
                                   get_default_cache_config(CacheType::DirectMapped));
  for (int level = 0; level < hierarchy_size; level++)
    configs[level].size = size_per_level * (1 << level);

  CacheHierarchy ch { configs };
  const uint64_t line_size = configs[0].line_size;
  const uint64_t touches_per_level { size_per_level / line_size };

  std::array<uint64_t, hierarchy_size + 1> misses { 0, 0, 0, 0 };
  std::array<uint64_t, hierarchy_size + 1> hits { 0, 0, 0, 0 };

  // Fill up L1 (all misses)
  for (uint64_t a = 0; a < size_per_level; a += line_size) ch.touch(a);

  for (int level = 1; level <= hierarchy_size; level++) {
    misses[level] = touches_per_level;
    REQUIRE(ch.getMisses(level) == misses[level]);
    REQUIRE(ch.getHits(level) == hits[level]);
  }


  // Fill up the remainder of L2 (all misses)
  for (uint64_t a = size_per_level; a < 2 * size_per_level; a += line_size) ch.touch(a);

  for (int level = 1; level <= hierarchy_size; level++) {
    misses[level] += touches_per_level;
    REQUIRE(ch.getMisses(level) == misses[level]);
    REQUIRE(ch.getHits(level) == hits[level]);
  }

  // Read the previous values (misses L1, but hits L2)
  for (uint64_t a = 0; a < size_per_level; a += line_size) ch.touch(a);
  misses[1] += touches_per_level;
  hits[2] += touches_per_level;

  for (int level = 1; level <= hierarchy_size; level++) {
    REQUIRE(ch.getMisses(level) == misses[level]);
    REQUIRE(ch.getHits(level) == hits[level]);
  }


  // Fill up the remainder of L3 (all misses)
  for (uint64_t a = 2 * size_per_level; a < 4 * size_per_level; a += line_size)
    ch.touch(a);

  for (int level = 1; level <= hierarchy_size; level++) {
    misses[level] += 2 * touches_per_level;
    REQUIRE(ch.getMisses(level) == misses[level]);
  }

  // Read the initial values (misses L1 and L2, but hits L3)
  for (uint64_t a = 0; a < size_per_level; a += line_size) ch.touch(a);
  misses[1] += touches_per_level;
  misses[2] += touches_per_level;
  hits[3] += touches_per_level;

  for (int level = 1; level <= hierarchy_size; level++) {
    REQUIRE(ch.getMisses(level) == misses[level]);
    REQUIRE(ch.getHits(level) == hits[level]);
  }
}

TEST_CASE("Sized access touched the correct number of cache lines thorough a hierarchy",
          "[hierarchy]") {
  const int lines_touched = GENERATE(range(1, 5));
  auto ch                 = make_default_hierarchy(CacheType::SetAssociative);

  const SizedAccess access = { 0, lines_touched * ch->getLineSize(1) };
  ch->touch(access);

  REQUIRE(ch->getMisses(1) == static_cast<uint64_t>(lines_touched));
  REQUIRE(ch->getHits(1) == 0);
}

TEST_CASE("Traffic between levels is counted correctly", "[hierarchy][stats]") {
  auto ch = make_default_hierarchy(CacheType::SetAssociative);

  SECTION("When accessing full lines") {
    const int size = GENERATE(8, 16, 64);

    // Miss both caches
    for (uint64_t address = 0; address < DEFAULT_CACHE_SIZE; address += size)
      ch->touch(address, size);

    // Miss L1 but not L2
    for (uint64_t address = 0; address < DEFAULT_CACHE_SIZE / 2; address += size)
      ch->touch(address, size);

    // Traffic between to the first level of cache depends on the size of the requests
    REQUIRE(ch->getTraffic(0) == DEFAULT_CACHE_SIZE / 2 * 3);
  }

  SECTION("When partially accessing lines") {

    // Miss both caches
    for (uint64_t address = 0; address < DEFAULT_CACHE_SIZE; address += DEFAULT_LINE_SIZE)
      ch->touch(address, 1);

    // Miss L1 but not L2
    for (uint64_t address = 0; address < DEFAULT_CACHE_SIZE / 2;
         address += DEFAULT_LINE_SIZE)
      ch->touch(address, 1);

    // Traffic between to the first level of cache depends on the size of the requests
    REQUIRE(ch->getTraffic(0) == DEFAULT_CACHE_SIZE / DEFAULT_LINE_SIZE / 2 * 3);
  }

  // Sanity check that we've touched the elements we wanted
  CHECK(ch->getMisses(1) ==
        3 * (DEFAULT_CACHE_SIZE / DEFAULT_HIERARCHY_SIZE) / DEFAULT_LINE_SIZE);
  CHECK(ch->getMisses(2) == DEFAULT_CACHE_SIZE / DEFAULT_LINE_SIZE);

  // Traffic between caches only depends on cache line sizes, not on the size of the accesses
  REQUIRE(ch->getTraffic(1) == DEFAULT_CACHE_SIZE / 2 * 3);
  REQUIRE(ch->getTraffic(2) == DEFAULT_CACHE_SIZE);
}
