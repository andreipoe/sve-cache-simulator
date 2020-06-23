#pragma once

#include <limits>
#include <memory>
#include <string>

#include "RandomAddressGenerator.hh"

#include "CacheHierarchy.hh"

#define DEFAULT_CACHE_SIZE 32 * 1024
#define DEFAULT_LINE_SIZE  64
#define DEFAULT_SET_SIZE   4

#define DEFAULT_HIERARCHY_SIZE 2

constexpr unsigned int nbits(const uint64_t n) { return n == 1 ? 0 : 1 + nbits(n >> 1); }

#define DEFAULT_BLOCK_BITS nbits(DEFAULT_LINE_SIZE)
#define DEFAULT_INDEX_BITS nbits(DEFAULT_CACHE_SIZE / DEFAULT_LINE_SIZE)
#define DEFAULT_TAG_BITS \
  nbits(DEFAULT_CACHE_SIZE) - DEFAULT_INDEX_BITS - DEFAULT_BLOCK_BITS

#define MAX_ADDRESS_INT static_cast<int>(std::numeric_limits<uint64_t>)

#define DEFAULT_RANDOM_COUNT 10


/* Try a few common locations where test trace files can be found, returning the first one
 * that matches or raising an exception
 * Deprecated: Use a TestTrace constant object instead of a test trace file */
std::string try_tracefile_names(const std::string& name);

/* Try a few common locations where test trace files can be found, returning the first one
 * that matches or raising an exception */
std::string try_configfile_names(const std::string& name);

/* Generate a random address */
uint64_t get_random_address();

/* Generate a set of unique random address */
std::vector<uint64_t> get_random_unique_addresses(int n, uint64_t except = 0);

/* Get the config for the specified cache type, using the default testing parameters */
CacheConfig get_default_cache_config(CacheType type);

/* Create a cache of the specified type, using the default testing parameters */
std::unique_ptr<Cache> make_default_cache(CacheType type);

/* Create a cache hierarchy cache of the specified type, using the default testing
 * parameters */
std::unique_ptr<CacheHierarchy> make_default_hierarchy(CacheType type);

/* Check whether two memory traces represent the same sequence of memory requests */
bool trace_equals(const MemoryTrace& trace1, const MemoryTrace& trace2);


namespace TestTraces {
const std::string BUNDLE =
    "4016116, 0, 3, 0, 8, 0x6cf540, 0x40e364\n"
    "4016117, 0, 2, 0, 8, 0x6cf560, 0x40e364\n"
    "4016118, 0, 2, 0, 8, 0x6cf580, 0x40e364\n"
    "4016123, 0, 6, 0, 8, 0x6cf620, 0x40e364\n"
    "4016116, 0, 3, 0, 8, 0x6cf540, 0x40e364\n"
    "4016117, 0, 2, 0, 8, 0x6cf560, 0x40e364\n"
    "4016118, 0, 2, 0, 8, 0x6cf580, 0x40e364\n"
    "4016123, 0, 6, 0, 8, 0x6cf620, 0x40e364\n"
    "4016124, 0, 0, 1, 64, 0x6e0000, 0x40e370\n"
    "4016126, 0, 0, 0, 64, 0x630a00, 0x40e360\n"
    "4016127, 0, 3, 0, 8, 0x6cf580, 0x40e200\n"
    "4016128, 0, 2, 0, 8, 0x6cf5a0, 0x40e200\n"
    "4016129, 0, 2, 0, 8, 0x6cf5c0, 0x40e200\n"
    "4016130, 0, 2, 0, 8, 0x6cf5e0, 0x40e200\n"
    "4016131, 0, 2, 0, 8, 0x6cf600, 0x40e200\n"
    "4016134, 0, 6, 0, 8, 0x6cf660, 0x40e200\n";

const std::string SIMPLE5 =
    "32, 0, 0, 0, 16, 0xffff37414010, 0x40091c\n"
    "33, 0, 0, 1, 16, 0xffff37313010, 0x400924\n"
    "4016116, 0, 3, 0, 8, 0x6cf540, 0x40e364\n"
    "4016118, 0, 2, 0, 8, 0x6cf580, 0x40e364\n"
    "4016123, 0, 6, 0, 8, 0x6cf620, 0x40e364\n";
}  // namespace TestTraces
