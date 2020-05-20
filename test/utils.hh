#pragma once

#include <limits>
#include <memory>
#include <string>

#include "RandomAddressGenerator.hh"

#include "cache.hh"

#define DEFAULT_CACHE_SIZE 32 * 1024
#define DEFAULT_LINE_SIZE  64
#define DEFAULT_SET_SIZE   4

constexpr unsigned int nbits(const uint64_t n) { return n == 1 ? 0 : 1 + nbits(n >> 1); }

#define DEFAULT_BLOCK_BITS nbits(DEFAULT_LINE_SIZE)
#define DEFAULT_INDEX_BITS nbits(DEFAULT_CACHE_SIZE / DEFAULT_LINE_SIZE)
#define DEFAULT_TAG_BITS \
  nbits(DEFAULT_CACHE_SIZE) - DEFAULT_INDEX_BITS - DEFAULT_BLOCK_BITS

#define MAX_ADDRESS_INT static_cast<int>(std::numeric_limits<uint64_t>)

#define DEFAULT_RANDOM_COUNT 10


/* Try a few common locations where test trace files can be found, returning the first one
 * that matches or raising an exception */
std::string try_tracefile_names(const std::string& name);

/* Try a few common locations where test trace files can be found, returning the first one
 * that matches or raising an exception */
std::string try_configfile_names(const std::string& name);

/* Generate a random address */
uint64_t get_random_address();

/* Generate a set of unique random address */
std::vector<uint64_t> get_random_unique_addresses(int n, uint64_t except = 0);

/* Create a cache of the specified type, using the default testing parameters */
std::unique_ptr<Cache> make_default_cache(CacheType type);
