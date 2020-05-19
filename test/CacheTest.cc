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

  // Generate addresses that map to the first element in a cache line
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

TEST_CASE("Accessing an unaligned value brings in a whole cache line") {
  auto type = GENERATE(CacheType::Infinite, CacheType::DirectMapped);
  std::unique_ptr<Cache> cache = make_default_cache(type);

  // Generate addresses that don't map to the first element in a cache line
  const uint64_t initial_address { static_cast<uint64_t>(
      GENERATE(take(RANDOM_COUNT, filter([](int n) { return n % 64 != 0; },
                                         random(0, DEFAULT_CACHE_SIZE))))) };

  cache->touch(initial_address);  // miss
  REQUIRE(cache->getHits() == 0);
  REQUIRE(cache->getMisses() == 1);

  const uint64_t block { initial_address & ((1 << nbits(DEFAULT_LINE_SIZE)) - 1) };
  assert(block > 0 && block < DEFAULT_LINE_SIZE && "Generated address is not unaligned");
  const uint64_t cache_line_start { initial_address - block };

  for (int offset = 0; offset < DEFAULT_LINE_SIZE; offset++)
    cache->touch(cache_line_start + offset);  // hits

  REQUIRE(cache->getHits() == DEFAULT_LINE_SIZE);
  REQUIRE(cache->getMisses() == 1);

  cache->touch(cache_line_start + DEFAULT_LINE_SIZE);  // miss
  REQUIRE(cache->getHits() == DEFAULT_LINE_SIZE);
  REQUIRE(cache->getMisses() == 2);
}

TEST_CASE("Accesses bigger than the size of a cache line touch multiple cache lines") {
  auto type = GENERATE(CacheType::Infinite, CacheType::DirectMapped);
  std::unique_ptr<Cache> cache = make_default_cache(type);

  // Generate addresses that map to the first element in a cache line
  const int r { GENERATE(
      take(RANDOM_COUNT, random(0, DEFAULT_CACHE_SIZE / DEFAULT_LINE_SIZE))) };
  const uint64_t base_address { static_cast<uint64_t>(r) * DEFAULT_LINE_SIZE };
  const int n_lines { 4 };

  cache->touch(base_address, n_lines * DEFAULT_LINE_SIZE);  // a miss for each line
  REQUIRE(cache->getHits() == 0);
  REQUIRE(cache->getMisses() == n_lines);

  for (int offset = 0; offset < n_lines; offset++)
    cache->touch(base_address + offset * DEFAULT_LINE_SIZE);  // hit
  REQUIRE(cache->getHits() == n_lines);
  REQUIRE(cache->getMisses() == n_lines);

  cache->touch(base_address + (n_lines + 1) * DEFAULT_LINE_SIZE);  // miss
  REQUIRE(cache->getHits() == n_lines);
  REQUIRE(cache->getMisses() == n_lines + 1);
}
