#include "catch.hpp"

#include "utils.hh"

#define RANDOM_COUNT 100

TEST_CASE("Infinite caches never evict", "[model][infinite]"){
  auto cache = make_default_cache(CacheType::Infinite);

  const uint64_t address = GENERATE(take(RANDOM_COUNT, random_addresses()));
  cache->touch(address);

  REQUIRE(cache->getEvictions() == 0);
}
