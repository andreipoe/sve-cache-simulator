#include "catch.hpp"

#include <vector>

#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"
#include "MemoryTrace.hh"
#include "utils.hh"

#define CACHE_SIZE DEFAULT_CACHE_SIZE
#define LINE_SIZE DEFAULT_LINE_SIZE

#define RANDOM_COUNT 10

// TODO: Use the same generator approach in all tests

TEST_CASE("Addresses are split correctly") {
  const CacheConfig config { CacheType::Infinite, CACHE_SIZE, LINE_SIZE };

  const unsigned int block { 32 }, index { 160 }, tag { 12 };
  const uint64_t address { tag << nbits(CACHE_SIZE) | index << nbits(LINE_SIZE) | block };

  const CacheAddress cache_address(address, config);
  REQUIRE(cache_address.tag == tag);
  REQUIRE(cache_address.index == index);
  REQUIRE(cache_address.tag == tag);
}

TEST_CASE("Cache stats are properly initialised") {
  auto type = GENERATE(CacheType::Infinite, CacheType::DirectMapped);
  std::unique_ptr<Cache> cache = make_default_cache(type);

  REQUIRE(cache->getHits() == 0);
  REQUIRE(cache->getMisses() == 0);
}

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
  uint64_t address { 0xdead };

  InfiniteCache ic;
  DirectMappedCache dmc({ CacheType::DirectMapped, CACHE_SIZE, LINE_SIZE });

  for (Cache* cache : std::vector<Cache*> { &ic, &dmc }) {
    cache->touch(address);
    cache->touch(address);

    REQUIRE(cache->getMisses() == 1);
    REQUIRE(cache->getHits() == 1);
  }
}

TEST_CASE("Accessing an aligned value brings in a whole cache line") {
  auto type = GENERATE(CacheType::Infinite, CacheType::DirectMapped);
  std::unique_ptr<Cache> cache = make_default_cache(type);

  // const uint64_t base_address { 0 };
  const int r { GENERATE(
      take(RANDOM_COUNT, random(0, DEFAULT_CACHE_SIZE / DEFAULT_LINE_SIZE))) };
  const uint64_t base_address { static_cast<uint64_t>(r) * DEFAULT_LINE_SIZE };

  cache->touch(base_address);  // miss
  REQUIRE(cache->getHits() == 0);
  REQUIRE(cache->getMisses() == 1);

  for (int offset = 0; offset < DEFAULT_LINE_SIZE; offset++)
    cache->touch(base_address + offset);  // hits

  REQUIRE(cache->getHits() == DEFAULT_LINE_SIZE);
  REQUIRE(cache->getMisses() == 1);

  cache->touch(base_address + DEFAULT_LINE_SIZE);  // miss
  REQUIRE(cache->getHits() == DEFAULT_LINE_SIZE);
  REQUIRE(cache->getMisses() == 2);
}
