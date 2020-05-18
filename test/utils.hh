#pragma once

#include <memory>
#include <string>

#include "cache.hh"

#define DEFAULT_CACHE_SIZE 32 * 1024
#define DEFAULT_LINE_SIZE 64

/* Try a few common locations where test trace files can be found, returning the first one
 * that matches or raising an exception */
std::string try_tracefile_names(const std::string& name);

/* Try a few common locations where test trace files can be found, returning the first one
 * that matches or raising an exception */
std::string try_configfile_names(const std::string& name);

constexpr unsigned int nbits(const uint64_t n) { return n == 1 ? 0 : 1 + nbits(n >> 1); }

std::unique_ptr<Cache> make_default_cache(CacheType type);
