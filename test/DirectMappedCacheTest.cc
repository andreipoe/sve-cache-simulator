#include "catch.hpp"

#include "utils.hh"

TEST_CASE("Conflicting memory addresses evict previous direct-mapped data") {
  const int RANDOM_COUNT { 2 };

  const uint64_t initial_address =
      GENERATE(take(RANDOM_COUNT, random(0, DEFAULT_CACHE_SIZE)));

  // Keep the index
  const uint64_t initial_index { (initial_address >> DEFAULT_BLOCK_BITS) &
                                 (1 << DEFAULT_INDEX_BITS) - 1 };

  // Generate a new tag and block id
  const uint64_t new_tag =
      GENERATE(take(RANDOM_COUNT, random(0, DEFAULT_CACHE_SIZE / DEFAULT_SET_SIZE /
                                                DEFAULT_LINE_SIZE)));
  const uint64_t new_block = GENERATE(take(RANDOM_COUNT, random(0, DEFAULT_LINE_SIZE)));

  const uint64_t conflicting_address =
      (new_tag << (DEFAULT_INDEX_BITS + DEFAULT_BLOCK_BITS)) |
      (initial_index << DEFAULT_BLOCK_BITS) | new_block;

  auto cache = make_default_cache(CacheType::DirectMapped);
  cache->touch(initial_address);
  cache->touch(conflicting_address);
  REQUIRE(cache->getHits() == 0);
  REQUIRE(cache->getMisses() == 2);
  REQUIRE(cache->getEvictions() == 1);
}
