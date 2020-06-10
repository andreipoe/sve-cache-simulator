#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include <getopt.h>

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
#define EXIT_UNKONWN_ENCODING  5

#define OPT_ENCODING_TEXT   1
#define OPT_ENCODING_BINARY 2

class CommaNumPunct : public std::numpunct<char> {
 protected:
  virtual char do_thousands_sep() const { return ','; }
  virtual std::string do_grouping() const { return "\03"; }
};

void usage(int code = 1);

TraceFileType guess_file_type(const std::string& fname);
void run_and_print_stats(MemoryTrace const& trace, Cache& cache);
void run_and_print_stats(MemoryTrace const& trace, CacheHierarchy& cache);


int main(int argc, char* argv[]) {
  // TODO: consider using Lyra for argument parsing https://github.com/bfgroup/Lyra
  int opt;
  std::string config_fname;
  bool encoding_provided { false };
  TraceFileType trace_encoding {};

  struct option long_options[] = { { "config", required_argument, NULL, 'c' },
                                   { "text", no_argument, NULL, OPT_ENCODING_TEXT },
                                   { "binary", no_argument, NULL, OPT_ENCODING_BINARY },
                                   { "help", no_argument, NULL, 'h' },
                                   { 0, 0, 0, 0 } };

  while ((opt = getopt_long(argc, argv, "c:h", long_options, NULL)) != -1) {
    switch (opt) {
      case 'c':
        config_fname = optarg;
        break;

      case OPT_ENCODING_TEXT:
        if (encoding_provided) usage(EXIT_INVALID_ARGUMENTS);
        encoding_provided = true;
        trace_encoding    = TraceFileType::Text;
        break;
      case OPT_ENCODING_BINARY:
        if (encoding_provided) usage(EXIT_INVALID_ARGUMENTS);
        encoding_provided = true;
        trace_encoding    = TraceFileType::Binary;
        break;

      case 'h':
        usage(0);
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

  if (!encoding_provided) trace_encoding = guess_file_type(argv[0]);

  std::cout << "Trace file encoding: ";
  switch (trace_encoding) {
    case TraceFileType::Text:
      std::cout << "text";
      break;
    case TraceFileType::Binary:
      std::cout << "binary";
      break;
    default:
      std::cout << "unknown\n";
      std::exit(EXIT_UNKONWN_ENCODING);
  }
  if (encoding_provided)
    std::cout << "\n";
  else
    std::cout << " (guessed)\n";

  std::ifstream tracefile { argv[0] };
  if (!tracefile.is_open()) {
    std::cout << "Cannot open trace file: " << argv[0] << "\n";
    std::exit(EXIT_INVALID_TRACE);
  }

  MemoryTrace trace { tracefile, trace_encoding };

  std::cout.imbue({ std::locale(), new CommaNumPunct() });

  CacheHierarchy cache { std::move(config_file) };
  run_and_print_stats(trace, cache);

  return 0;
}


void usage(int code) {
  std::cout << "Usage:\n";
  std::cout << "  scs --binary -c CONFIG-FILE TRACE-FILE\n";
  std::cout << "  scs --text   -c CONFIG-FILE TRACE-FILE\n";
  std::cout << "  scs --help\n";
  std::exit(code);
}


/* Guess if the given file is a text file */
TraceFileType guess_file_type(const std::string& fname) {
  std::ifstream f { fname };
  const size_t check_count { 500 };

  std::unique_ptr<char> data { new char[check_count + 1] };
  f.read(data.get(), check_count);

  if (std::memchr(data.get(), '\0', check_count) != NULL)
    return TraceFileType::Binary;
  else
    return TraceFileType::Text;
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

  std::ostringstream csv_data, csv_header;

  const auto addresses = trace.getRequestAddresses();
  std::set<uint64_t> unique_addresses(addresses.begin(), addresses.end());
  std::cout << "Trace has " << trace.getLength() << " entries.\n";
  std::cout << "Seen " << unique_addresses.size() << " unique addresses.\n\n";

  std::cout << "CPU to L1 traffic: " << cache.getTraffic(0) << " bytes\n";

  // TODO: Add flags to choose between plain and CSV output. Batch experiments should only
  // output CSV
  csv_header << "CPU-L1-traffic,";
  csv_data << cache.getTraffic(0) << ',';

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

    csv_header << level_names[level] << "-miss-pct," << level_names[level]
               << "-evictions," << level_names[level] << '-' << level_names[level + 1]
               << "-traffic,";
    csv_data << pct_misses << ',' << evictions << ',' << cache.getTraffic(level) << ',';
  }

  const auto bundles = cache.getBundleOps();
  uint64_t total_bundles { 0 }, total_bundle_ops { 0 };
  for (const auto b : bundles) {
    total_bundles += b.second.times_encountered;
    total_bundle_ops += b.second.total_ops;
  }
  const auto bundle_ratio =
      static_cast<double>(total_bundle_ops) / addresses.size() * 100;

  std::cout << "\n";
  std::cout << "Total scatter/gather bundles simulated: " << total_bundles << "\n";
  std::cout << "Total unique scatter/gather bundles encountered: " << bundles.size()
            << "\n";
  std::cout << "Total ops part of scatters/gathers: " << total_bundle_ops << " ("
            << std::setprecision(2) << bundle_ratio << "%)\n";

  std::cout << "\n" << csv_header.str() << "\n" << csv_data.str() << "\n";
}
