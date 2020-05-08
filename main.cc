#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "InfiniteCache.hh"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: scs TRACE-FILE\n";
    return 1;
  }

  size_t count = 0;
  std::map<int, size_t> addresses;
  InfiniteCache cache;

  std::ifstream trace(argv[1]);
  for (std::string line; std::getline(trace, line);) {
    std::remove(line.begin(), line.end(), ',');

#ifdef DEBUG
    std::cout << "Line: " << line << "\n";
#endif

    std::istringstream iss(line);
    int seq, tid, size;
    bool is_bundle, is_write;
    long address, pc;
    iss >> seq >> tid >> is_bundle >> is_write >> size;
    iss >> std::hex >> address >> pc;

#ifdef DEBUG
    std::cout << "Values: " << seq << " " << tid << " " << is_bundle << " "
              << is_write << " " << size << " " << address << " " << pc << "\n";
#endif

    count++;
    addresses[address]++;
    cache.touch(address);
  }

  std::cout << "Processed " << count << " lines.\n";
  std::cout << "Seen " << addresses.size() << " unique addresses.\n\n";

  auto hits = cache.getHits();
  auto misses = cache.getMisses();
  auto total = cache.getTotalAccesses();
  auto pct_hits = (hits / total) * 100.0;
  auto pct_misses = 100.0 - pct_hits;
  std::cout << "Total accesses: " << total << "\n";
  std::cout << "Hits: " << hits << " (" << std::fixed << std::setprecision(2)
            << pct_hits << "%)\n";
  std::cout << "Misses: " << misses << " (" << std::fixed
            << std::setprecision(2) << pct_misses << "%)\n";

  return 0;
}
