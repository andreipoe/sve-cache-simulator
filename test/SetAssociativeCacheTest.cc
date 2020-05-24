#include "catch.hpp"

#include <algorithm>

#include "utils.hh"

TEST_CASE("Conflicting memory addresses evict previous set-associative data", "[model][set-associative]") {

  // Generate an address
  auto cache = make_default_cache(CacheType::SetAssociative);
  const CacheAddress initial_address {
    GENERATE(take(DEFAULT_RANDOM_COUNT, random<uint64_t>(0, -1))), *cache
  };

  // Make more addresses with the same index, but different tags
  std::vector<CacheAddress> set_conflicts(DEFAULT_SET_SIZE, initial_address);
  for (size_t i = 0; i < DEFAULT_SET_SIZE; i++) {
    set_conflicts[i].tag   = initial_address.tag + i + 1;
    set_conflicts[i].block = initial_address.block + i + 1;
  }

  cache->touch(initial_address);  // miss, no eviction
  REQUIRE(cache->getHits() == 0);

  for (size_t i = 0; i < DEFAULT_SET_SIZE - 1; i++)
    cache->touch(set_conflicts[i]);                   // miss, no eviction
  cache->touch(set_conflicts[DEFAULT_SET_SIZE - 1]);  // miss, eviction

  REQUIRE(cache->getHits() == 0);
  REQUIRE(cache->getMisses() == 1 + DEFAULT_SET_SIZE);
  REQUIRE(cache->getEvictions() == 1);
}

// TODO: Test different numbers of ways work as expected
