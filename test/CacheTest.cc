#include "catch.hpp"

#include <vector>

#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"
#include "MemoryTrace.hh"
#include "utils.hh"

#define CACHE_SIZE 32 * 1024
#define LINE_SIZE 64

TEST_CASE("First touch always misses") {
  auto filename = try_tracefile_names("traces/7.trace");

  std::ifstream tracefile(filename);
  MemoryTrace trace(tracefile);

  SECTION("Infinite cache") {
    InfiniteCache cache;
    cache.touch(trace.getRequestAddresses());

    REQUIRE(cache.getHits() == 0);
  }

  SECTION("Direct-mapped cache") {
    DirectMappedCache cache({ CacheType::DirectMapped, CACHE_SIZE, LINE_SIZE });
    cache.touch(trace.getRequestAddresses());

    REQUIRE(cache.getHits() == 0);
  }
}

TEST_CASE("Second touch always hits") {
  long address { 0xdead };

  InfiniteCache ic;
  DirectMappedCache dmc({ CacheType::DirectMapped, CACHE_SIZE, LINE_SIZE });

  for (Cache* cache : std::vector<Cache*> { &ic, &dmc }) {
    cache->touch(address);
    cache->touch(address);

    REQUIRE(cache->getMisses() == 1);
    REQUIRE(cache->getHits() == 1);
  }
}