#include "catch.hpp"

#include <limits>
#include <vector>

#include "utils.hh"

#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"
#include "MemoryTrace.hh"

#define RANDOM_COUNT DEFAULT_RANDOM_COUNT

auto const CACHE_TYPES = { CacheType::Infinite, CacheType::DirectMapped,
                           CacheType::SetAssociative };


TEST_CASE("Addresses are split correctly", "[model][common][addresses]") {
  const CacheConfig config { CacheType::Infinite, DEFAULT_CACHE_SIZE, DEFAULT_LINE_SIZE };

  const unsigned int block { 32 }, index { 160 }, tag { 12 };
  const uint64_t address { tag << nbits(DEFAULT_CACHE_SIZE) |
                           index << nbits(DEFAULT_LINE_SIZE) | block };

  const CacheAddress cache_address(address, config);
  REQUIRE(cache_address.tag == tag);
  REQUIRE(cache_address.index == index);
  REQUIRE(cache_address.tag == tag);
}

TEST_CASE("Cache type is returned correctly", "[model][common]") {
  const CacheType requested_type = GENERATE(values(CACHE_TYPES));
  std::unique_ptr<Cache> cache = make_default_cache(requested_type);

  REQUIRE(cache->getType() == requested_type);
}

TEST_CASE("Cache stats are properly initialised", "[model][common][stats]") {
  std::unique_ptr<Cache> cache = make_default_cache(GENERATE(values(CACHE_TYPES)));

  REQUIRE(cache->getHits() == 0);
  REQUIRE(cache->getMisses() == 0);
  REQUIRE(cache->getTotalAccesses() == 0);
  REQUIRE(cache->getEvictions() == 0);
}

TEST_CASE("Hits and misses always add up to total touches", "[model][common][stats]") {
  const int TOUCH_COUNT { 1000 };

  std::unique_ptr<Cache> cache = make_default_cache(GENERATE(values(CACHE_TYPES)));

  const auto& addresses = GENERATE(
      take(1, chunk(TOUCH_COUNT,
                    random(0, static_cast<int>(std::numeric_limits<uint64_t>::max())))));

  for (auto address : addresses)
    cache->touch(address);

  REQUIRE(cache->getTotalAccesses() == TOUCH_COUNT);
  REQUIRE(cache->getHits() + cache->getMisses() == TOUCH_COUNT);
}

TEST_CASE("First touch always misses", "[model][common]") {
  const uint64_t address { static_cast<uint64_t>(
      GENERATE(take(RANDOM_COUNT, random(0, DEFAULT_CACHE_SIZE)))) };

  std::unique_ptr<Cache> cache = make_default_cache(GENERATE(values(CACHE_TYPES)));

  cache->touch(address);
  REQUIRE(cache->getHits() == 0);
  REQUIRE(cache->getMisses() == 1);
}

TEST_CASE("Second touch always hits", "[model][common]") {
  const uint64_t address { static_cast<uint64_t>(
      GENERATE(take(RANDOM_COUNT, random(0, DEFAULT_CACHE_SIZE)))) };

  std::unique_ptr<Cache> cache = make_default_cache(GENERATE(values(CACHE_TYPES)));

  cache->touch(address);  // miss
  cache->touch(address);  // hit

  REQUIRE(cache->getMisses() == 1);
  REQUIRE(cache->getHits() == 1);
}

TEST_CASE("Accessing an aligned value brings in a whole cache line", "[model][common]") {
  std::unique_ptr<Cache> cache = make_default_cache(GENERATE(values(CACHE_TYPES)));

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

TEST_CASE("Accessing an unaligned value brings in a whole cache line", "[model][common]") {
  std::unique_ptr<Cache> cache = make_default_cache(GENERATE(values(CACHE_TYPES)));

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

TEST_CASE("Accesses bigger than the size of a cache line touch multiple cache lines", "[model][common]") {
  std::unique_ptr<Cache> cache = make_default_cache(GENERATE(values(CACHE_TYPES)));

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
