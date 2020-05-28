#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include <unistd.h>

#include "CacheConfig.hh"
#include "CacheHierarchy.hh"
#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"
#include "MemoryTrace.hh"
#include "SetAssociativeCache.hh"

#define EXIT_INVALID_OPTION    1
#define EXIT_INVALID_ARGUMENTS 2
#define EXIT_INVALID_CONFIG    3
#define EXIT_INVALID_TRACE     4

class CommaNumPunct : public std::numpunct<char> {
 protected:
  virtual char do_thousands_sep() const { return ','; }
  virtual std::string do_grouping() const { return "\03"; }
};

void usage(int code = 1);

void run_and_print_stats(MemoryTrace const& trace, Cache& cache);
void run_and_print_stats(MemoryTrace const& trace, CacheHierarchy& cache);

int main(int argc, char* argv[]) {
  // TODO: consider using Lyra for argument parsing https://github.com/bfgroup/Lyra
  char opt;
  std::string config_fname;

  while ((opt = getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
      case 'c':
        config_fname = optarg;
        break;

      case '?':
      default:
        usage(EXIT_INVALID_OPTION);
    }
  }
  argc -= optind;
  argv += optind;

  if (config_fname.empty() || argc < 1) {
    usage(EXIT_INVALID_ARGUMENTS);
  }

  std::ifstream config_file { config_fname };
  if (!config_file.is_open()) {
    std::cout << "Cannot open config file: " << config_fname << "\n";
    std::exit(EXIT_INVALID_CONFIG);
  }

  std::ifstream tracefile { argv[0] };
  if (!tracefile.is_open()) {
    std::cout << "Cannot open trace file: " << argv[0] << "\n";
    std::exit(EXIT_INVALID_TRACE);
  }

  MemoryTrace trace(tracefile);

  std::cout.imbue({ std::locale(), new CommaNumPunct() });

  std::cout << "Using an infinite cache.\n";
  InfiniteCache infinite_cache;
  run_and_print_stats(trace, infinite_cache);

  std::cout << "--------------\n";

  std::cout << "Using a direct-mapped cache.\n";
  CacheConfig dm_config(CacheType::DirectMapped, 32768, 64);
  DirectMappedCache direct_mapped_cache(dm_config);
  run_and_print_stats(trace, direct_mapped_cache);

  std::cout << "--------------\n";

  std::cout << "Using a small set-associative cache.\n";
  CacheConfig sa_config(CacheType::SetAssociative, 32768, 64, 4);
  SetAssociativeCache set_associative_cache(sa_config);
  run_and_print_stats(trace, set_associative_cache);

  std::cout << "--------------\n";

  std::cout << "Using a TX2 cache.\n";
  CacheHierarchy tx2_cache { std::move(config_file) };
  run_and_print_stats(trace, tx2_cache);

  return 0;
}


void usage(int code) {
  std::cout << "Usage: scs -c CONFIG-FILE TRACE-FILE\n";
  std::exit(code);
}


void run_and_print_stats(MemoryTrace const& trace, Cache& cache) {
  auto addresses = trace.getRequestAddresses();
  cache.touch(addresses);

  std::set<uint64_t> unique_addresses(addresses.begin(), addresses.end());
  std::cout << "Trace has " << trace.getLength() << " entries.\n";
  std::cout << "Seen " << unique_addresses.size() << " unique addresses.\n\n";

  const auto hits       = cache.getHits();
  const auto misses     = cache.getMisses();
  const auto total      = cache.getTotalAccesses();
  const auto evictions  = cache.getEvictions();
  const auto pct_hits   = (static_cast<double>(hits) / total) * 100.0;
  const auto pct_misses = 100.0 - pct_hits;
  std::cout << "Total accesses: " << total << "\n";
  std::cout << "Hits: " << hits << " (" << std::fixed << std::setprecision(2) << pct_hits
            << "%)\n";
  std::cout << "Misses: " << misses << " (" << std::fixed << std::setprecision(2)
            << pct_misses << "%)\n";
  std::cout << "Evictions: " << evictions << "\n";
}

void run_and_print_stats(MemoryTrace const& trace, CacheHierarchy& cache) {
  auto addresses = trace.getRequestAddresses();
  cache.touch(addresses);

  std::set<uint64_t> unique_addresses(addresses.begin(), addresses.end());
  std::cout << "Trace has " << trace.getLength() << " entries.\n";
  std::cout << "Seen " << unique_addresses.size() << " unique addresses.\n";

  for (int level = 1; level <= cache.nlevels(); level++) {
    const auto hits       = cache.getHits(level);
    const auto misses     = cache.getMisses(level);
    const auto total      = cache.getTotalAccesses(level);
    const auto evictions  = cache.getEvictions(level);
    const auto pct_hits   = (static_cast<double>(hits) / total) * 100.0;
    const auto pct_misses = 100.0 - pct_hits;

    const std::string levelname { "L" + std::to_string(level) };
    std::cout << "\n";
    std::cout << levelname << " Total accesses: " << total << "\n";
    std::cout << levelname << " Hits: " << hits << " (" << std::fixed
              << std::setprecision(2) << pct_hits << "%)\n";
    std::cout << levelname << " Misses: " << misses << " (" << std::fixed
              << std::setprecision(2) << pct_misses << "%)\n";
    std::cout << levelname << " Evictions: " << evictions << "\n";
  }
}
