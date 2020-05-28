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
  std::cout << "Config file: " << config_fname << "\n";

  std::ifstream tracefile { argv[0] };
  if (!tracefile.is_open()) {
    std::cout << "Cannot open trace file: " << argv[0] << "\n";
    std::exit(EXIT_INVALID_TRACE);
  }
  MemoryTrace trace(tracefile);

  std::cout.imbue({ std::locale(), new CommaNumPunct() });

  CacheHierarchy cache { std::move(config_file) };
  run_and_print_stats(trace, cache);

  return 0;
}


void usage(int code) {
  std::cout << "Usage: scs -c CONFIG-FILE TRACE-FILE\n";
  std::exit(code);
}


// Deprecated
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
  cache.touch(trace.getRequests());

  const auto addresses = trace.getRequestAddresses();
  std::set<uint64_t> unique_addresses(addresses.begin(), addresses.end());
  std::cout << "Trace has " << trace.getLength() << " entries.\n";
  std::cout << "Seen " << unique_addresses.size() << " unique addresses.\n\n";

  std::cout << "CPU to L1 traffic: " << cache.getTraffic(0) << " bytes\n";

  std::vector<std::string> level_names(cache.nlevels() + 2);
  for (int level = 1; level <= cache.nlevels() + 1; level++)
    level_names[level] = "L" + std::to_string(level);

  for (int level = 1; level <= cache.nlevels(); level++) {
    const auto hits       = cache.getHits(level);
    const auto misses     = cache.getMisses(level);
    const auto total      = cache.getTotalAccesses(level);
    const auto evictions  = cache.getEvictions(level);
    const auto pct_hits   = (static_cast<double>(hits) / total) * 100.0;
    const auto pct_misses = 100.0 - pct_hits;

    std::cout << "\n";
    std::cout << level_names[level] << " Total accesses: " << total << "\n";
    std::cout << level_names[level] << " Hits: " << hits << " (" << std::fixed
              << std::setprecision(2) << pct_hits << "%)\n";
    std::cout << level_names[level] << " Misses: " << misses << " (" << std::fixed
              << std::setprecision(2) << pct_misses << "%)\n";
    std::cout << level_names[level] << " Evictions: " << evictions << "\n";
    std::cout << level_names[level] << " to " << level_names[level + 1]
              << " traffic: " << cache.getTraffic(level) << " bytes\n";
  }
}
