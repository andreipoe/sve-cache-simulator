#include "catch.hpp"

#include <algorithm>

#include "utils.hh"

TEST_CASE("Set size must divide total cache size", "[model][set-associative]") {
  const int set_size = GENERATE(take(
      DEFAULT_RANDOM_COUNT, filter([](int n) { return n % DEFAULT_CACHE_SIZE != 0; },
                                   random(0, DEFAULT_CACHE_SIZE / DEFAULT_LINE_SIZE))));
  const CacheConfig config { CacheType::SetAssociative, DEFAULT_CACHE_SIZE,
                             DEFAULT_LINE_SIZE, set_size };

  REQUIRE_THROWS_WITH(Cache::make_cache(config, std::make_shared<Clock>()),
                      "Set size does not divide cache size");
}

TEST_CASE("Conflicting memory addresses evict previous set-associative data",
          "[model][set-associative]") {
  auto const associativity = GENERATE(2, 4, 8, 16);
  auto config              = get_default_cache_config(CacheType::SetAssociative);
  config.set_size          = associativity;

  auto cache = Cache::make_cache(config, std::make_shared<Clock>());

  // Generate an address
  const CacheAddress initial_address {
    GENERATE(take(DEFAULT_RANDOM_COUNT, random<uint64_t>(0, -1))), *cache
  };

  // Make more addresses with the same index, but different tags
  std::vector<CacheAddress> set_conflicts(associativity, initial_address);
  for (int i = 0; i < associativity; i++) {
    set_conflicts[i].tag   = initial_address.tag + i + 1;
    set_conflicts[i].block = initial_address.block + i + 1;
  }

  cache->touch(initial_address);  // miss, no eviction
  REQUIRE(cache->getHits() == 0);

  for (int i = 0; i < associativity - 1; i++)
    cache->touch(set_conflicts[i]);                // miss, no eviction
  cache->touch(set_conflicts[associativity - 1]);  // miss, eviction

  REQUIRE(cache->getHits() == 0);
  REQUIRE(cache->getMisses() == static_cast<uint64_t>(1) + associativity);
  REQUIRE(cache->getEvictions() == 1);
}
