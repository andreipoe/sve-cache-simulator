#include "catch.hpp"

#include <fstream>
#include <typeinfo>

#include "CacheConfig.hh"
#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"
#include "utils.hh"

TEST_CASE("Reading cache configuration from ini files works") {
  std::string filename;
  CacheType expected_type;
  int expected_associativity;

  SECTION("Defining direct-mapped caches works") {
    filename      = try_configfile_names("direct-32KB.ini");
    expected_type = CacheType::DirectMapped;
    expected_associativity = 1;
  }
  SECTION("Defining set-associative caches works") {
    filename      = try_configfile_names("assoc-32KB.ini");
    expected_type = CacheType::SetAssociative;
    expected_associativity = 4;
  }

  CacheConfig c((std::ifstream(filename)));

  REQUIRE(c.size == 32 * 1024);
  REQUIRE(c.line_size == 64);
  REQUIRE(c.type == expected_type);
  REQUIRE(c.set_size == expected_associativity);
}

TEST_CASE("make_cache makes the right type of cache") {
  std::unique_ptr<Cache> ic = Cache::make_cache({ CacheType::Infinite, 0, 0 });
  const auto& ic_ref        = *ic.get();
  REQUIRE(typeid(ic_ref).hash_code() == typeid(InfiniteCache).hash_code());

  std::unique_ptr<Cache> dmc = Cache::make_cache({ CacheType::DirectMapped, 1024, 64 });
  const auto& dmc_ref        = *dmc.get();
  REQUIRE(typeid(dmc_ref).hash_code() == typeid(DirectMappedCache).hash_code());
}

TEST_CASE("Constructed caches have parameters given in CacheConfig") {
  int size { 4 * 1024 }, line_size { 512 };
  const CacheConfig config { CacheType::DirectMapped, size, line_size };
  const DirectMappedCache cache(config);

  REQUIRE(cache.getSize() == size);
  REQUIRE(cache.getLineSize() == line_size);
}
