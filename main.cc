#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "CacheConfig.hh"
#include "DirectMappedCache.hh"
#include "InfiniteCache.hh"
#include "MemoryTrace.hh"

void run_and_print_stats(MemoryTrace const& trace, Cache& cache) {
  auto addresses = trace.getRequestAddresses();
  cache.touch(addresses);

  std::set<long> unique_addresses(addresses.begin(), addresses.end());
  std::cout << "Trace has " << trace.getLength() << " entries.\n";
  std::cout << "Seen " << unique_addresses.size() << " unique addresses.\n\n";

  auto hits = cache.getHits();
  auto misses = cache.getMisses();
  auto total = cache.getTotalAccesses();
  auto pct_hits = (hits / total) * 100.0;
  auto pct_misses = 100.0 - pct_hits;
  std::cout << "Total accesses: " << total << "\n";
  std::cout << "Hits: " << hits << " (" << std::fixed << std::setprecision(2) << pct_hits
            << "%)\n";
  std::cout << "Misses: " << misses << " (" << std::fixed << std::setprecision(2)
            << pct_misses << "%)\n";
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: scs TRACE-FILE\n";
    return 1;
  }

  std::ifstream tracefile(argv[1]);
  MemoryTrace trace(tracefile);

  InfiniteCache infinite_cache;
  std::cout << "Using an infinite cache.\n";
  run_and_print_stats(trace, infinite_cache);

  std::cout << "--------------\n";

  CacheConfig config(std::ifstream("configs/32KB.ini"));
  DirectMappedCache direct_mapped_cache(config);
  std::cout << "Using a direct-mapped cache.\n";
  run_and_print_stats(trace, direct_mapped_cache);

  return 0;
}
