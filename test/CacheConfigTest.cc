#include "catch.hpp"

#include <fstream>
#include <sstream>
#include <typeinfo>

#include "utils.hh"

#include "CacheConfig.hh"
#include "CacheHierarchy.hh"
#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"

TEST_CASE("Reading cache configuration from ini files works", "[config][params]") {
  std::string filename;
  CacheType expected_type;
  int expected_associativity;

  SECTION("Defining direct-mapped caches works") {
    filename               = try_configfile_names("direct-32KB.ini");
    expected_type          = CacheType::DirectMapped;
    expected_associativity = 1;
  }
  SECTION("Defining set-associative caches works") {
    filename               = try_configfile_names("assoc-32KB.ini");
    expected_type          = CacheType::SetAssociative;
    expected_associativity = 4;
  }

  CacheConfig c { (std::ifstream(filename)) };

  REQUIRE(c.size == 32 * 1024);
  REQUIRE(c.line_size == 64);
  REQUIRE(c.type == expected_type);
  REQUIRE(c.set_size == expected_associativity);
}

TEST_CASE("Reading cache configuration from parameter maps works", "[config][params]") {
  const ConfigMap config_map {
    { "type", "set_associative" },
    { "cache_size", "8192" },
    { "line_size", "128" },
    { "set_size", "8" },
  };

  CacheConfig c { config_map };

  REQUIRE(c.type == CacheType::SetAssociative);
  REQUIRE(c.size == 8192);
  REQUIRE(c.line_size == 128);
  REQUIRE(c.set_size == 8);
}

TEST_CASE("make_cache makes the right type of cache", "[config][utils]") {
  std::unique_ptr<Cache> ic = Cache::make_cache({ CacheType::Infinite, 0, 0 });
  const auto& ic_ref        = *ic.get();
  REQUIRE(typeid(ic_ref).hash_code() == typeid(InfiniteCache).hash_code());

  std::unique_ptr<Cache> dmc = Cache::make_cache({ CacheType::DirectMapped, 1024, 64 });
  const auto& dmc_ref        = *dmc.get();
  REQUIRE(typeid(dmc_ref).hash_code() == typeid(DirectMappedCache).hash_code());
}

TEST_CASE("Constructed caches have parameters given in CacheConfig", "[config]") {
  int size { 4 * 1024 }, line_size { 512 };
  const CacheConfig config { CacheType::DirectMapped, size, line_size };
  const DirectMappedCache cache(config);

  REQUIRE(cache.getSize() == size);
  REQUIRE(cache.getLineSize() == line_size);
}

// TODO: Config files with a single level don't produce a hierarchy
// TEST_CASE("Config files with a single-level hierarchy produce a single cache",
// "[config]") {
// TODO: but throw if the section names aren't right
// }

TEST_CASE(
    "Attempting to construct a single cache from a multi-level hierarchy configuration "
    "throws",
    "[config]") {
  const auto filename = try_configfile_names("assoc-4+32KB.ini");
  REQUIRE_THROWS_AS(CacheConfig { std::ifstream { filename } }, std::invalid_argument);

  using Catch::Matchers::StartsWith;

  std::istringstream too_many_levels("[hierarchy]\nlevels = 2\n\n[level1]\nsize = 15");
  REQUIRE_THROWS_WITH(CacheConfig(std::move(too_many_levels)),
                      StartsWith("Too many levels for a single cache"));
}


TEST_CASE("Constructed cache hierarchies respect parameters in ini files",
          "[config][hierarchy]") {
  const auto filename = try_configfile_names("assoc-4+32KB.ini");
  const CacheHierarchy ch { std::ifstream(filename) };

  REQUIRE(ch.nlevels() == 2);

  REQUIRE(ch.getSize(1) == 4096);
  REQUIRE(ch.getLineSize(1) == 64);
  REQUIRE(ch.getSetSize(1) == 4);

  REQUIRE(ch.getSize(2) == 32768);
  REQUIRE(ch.getLineSize(2) == 64);
  REQUIRE(ch.getSetSize(2) == 4);
}

// TODO: Stats of caches in a hierarchy are initialised properly
