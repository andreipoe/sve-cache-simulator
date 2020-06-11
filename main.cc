#include <algorithm>
#include <bitset>
#include <chrono>
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

namespace {

#define EXIT_INVALID_OPTION    1
#define EXIT_INVALID_ARGUMENTS 2
#define EXIT_INVALID_CONFIG    3
#define EXIT_INVALID_TRACE     4
#define EXIT_UNKOWN_ENCODING   5
#define EXIT_OLD_CONFIG        6

#define OPT_ENCODING_TEXT   1
#define OPT_ENCODING_BINARY 2

enum OutputFormatBits {
  BIT_OUTPUT_TEXT,
  BIT_OUTPUT_CSV,

  OUTPUT_BIT_COUNT
};

using OutputFormat = std::bitset<OUTPUT_BIT_COUNT>;

class CommaNumPunct : public std::numpunct<char> {
 protected:
  virtual char do_thousands_sep() const { return ','; }
  virtual std::string do_grouping() const { return "\03"; }
};

void usage(int code) {
  // clang-format off
  std::cout << "Usage:\n";
  std::cout << "  scs --binary [OPTIONS] -c CONFIG-FILE TRACE-FILE\n";
  std::cout << "  scs --text   [OPTIONS] -c CONFIG-FILE TRACE-FILE\n";
  std::cout << "  scs --help\n\n";
  std::cout << "Options:\n";
  std::cout << "  -c CONFIG-FILE                The cache configuration file. Required at least once.\n";
  std::cout << "                                May be specified more than once for batched runs.\n";
  std::cout << "  -f, --format {text,csv,both}  Set the output format. Only 'csv' can be used with batches.\n";
  std::cout << "                                Default: 'both' for single runs, 'csv' for batches.\n";
  std::cout << "  -t, --timings                 Report run times of the main stages.\n";
  // clang-format on

  std::exit(code);
}

}  // namespace

TraceFileType guess_file_type(const std::string& fname);
void run_and_print_stats(MemoryTrace const& trace, CacheHierarchy& cache,
                         const OutputFormat& fmt, std::string_view config_fname);

using timestamp = std::chrono::time_point<std::chrono::high_resolution_clock>;
void print_timings(timestamp start, timestamp parse_end, timestamp simulation_end);


int main(int argc, char* argv[]) {
  int opt;
  std::string config_fname;
  bool encoding_provided { false }, enable_timing { false };
  TraceFileType trace_encoding {};
  OutputFormat output_format { (1 << OUTPUT_BIT_COUNT) - 1 };

  struct option long_options[] = { { "config", required_argument, NULL, 'c' },
                                   { "text", no_argument, NULL, OPT_ENCODING_TEXT },
                                   { "binary", no_argument, NULL, OPT_ENCODING_BINARY },
                                   { "format", required_argument, NULL, 'f' },
                                   { "timings", no_argument, NULL, 't' },
                                   { "help", no_argument, NULL, 'h' },
                                   { 0, 0, 0, 0 } };

  while ((opt = getopt_long(argc, argv, "c:f:th", long_options, NULL)) != -1) {
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

      case 'f':
        if (!strcmp(optarg, "text"))
          output_format.reset(BIT_OUTPUT_CSV);
        else if (!strcmp(optarg, "csv"))
          output_format.reset(BIT_OUTPUT_TEXT);
        else if (strcmp(optarg, "both") != 0)
          usage(EXIT_INVALID_ARGUMENTS);
        break;
      case 't':
        enable_timing = true;
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

  std::ostringstream info_output;

  std::ifstream config_file { config_fname };
  if (!config_file.is_open()) {
    std::cout << "Cannot open config file: " << config_fname << "\n";
    std::exit(EXIT_INVALID_CONFIG);
  }

  if (!encoding_provided) trace_encoding = guess_file_type(argv[0]);

  info_output << "Trace file encoding: ";
  switch (trace_encoding) {
    case TraceFileType::Text:
      info_output << "text";
      break;
    case TraceFileType::Binary:
      info_output << "binary";
      break;
    default:
      info_output << "unknown\n";
      std::exit(EXIT_UNKOWN_ENCODING);
  }
  if (encoding_provided)
    info_output << "\n";
  else
    info_output << " (guessed)\n";

  if (output_format[BIT_OUTPUT_TEXT]) std::cout << info_output.str();

  std::ifstream tracefile { argv[0] };
  if (!tracefile.is_open()) {
    std::cout << "Cannot open trace file: " << argv[0] << "\n";
    std::exit(EXIT_INVALID_TRACE);
  }

  const timestamp t_start = std::chrono::high_resolution_clock::now();

  MemoryTrace trace { tracefile, trace_encoding };

  const timestamp t_parse_end = std::chrono::high_resolution_clock::now();

  CacheHierarchy cache { std::move(config_file) };
  run_and_print_stats(trace, cache, output_format, config_fname);

  const timestamp t_sim_end = std::chrono::high_resolution_clock::now();

  if (enable_timing) print_timings(t_start, t_parse_end, t_sim_end);

  return 0;
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


void run_and_print_stats(MemoryTrace const& trace, CacheHierarchy& cache,
                         const OutputFormat& fmt, std::string_view config_fname) {
  cache.touch(trace.getRequests());

  std::ostringstream csv_data, csv_header;
  const auto addresses = trace.getRequestAddresses();

  if (fmt[BIT_OUTPUT_TEXT]) {
    std::cout.imbue({ std::locale(), new CommaNumPunct() });
    std::cout << "Config file: " << config_fname << "\n";

    std::set<uint64_t> unique_addresses(addresses.begin(), addresses.end());
    std::cout << "Trace has " << trace.getLength() << " entries.\n";
    std::cout << "Seen " << unique_addresses.size() << " unique addresses.\n";

    std::cout << "CPU to L1 traffic: " << cache.getTraffic(0) << " bytes\n";
  }

  // The config name is the base file name without the extension
  const auto last_slash  = config_fname.rfind('/');
  const auto last_dot    = config_fname.rfind('.');
  const auto config_name = config_fname.substr(last_slash + 1, last_dot - last_slash - 1);

  csv_header << "config,CPU-L1-traffic,";
  csv_data << config_name << ',' << cache.getTraffic(0) << ',';

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

    if (fmt[BIT_OUTPUT_TEXT]) {
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

  if (fmt[BIT_OUTPUT_TEXT]) {
    std::cout << "\n";
    std::cout << "Total scatter/gather bundles simulated: " << total_bundles << "\n";
    std::cout << "Total unique scatter/gather bundles encountered: " << bundles.size()
              << "\n";
    std::cout << "Total ops part of scatters/gathers: " << total_bundle_ops << " ("
              << std::setprecision(2) << bundle_ratio << "%)\n";
  }

  if (fmt[BIT_OUTPUT_CSV]) {
    if (fmt[BIT_OUTPUT_TEXT]) std::cout << "----------\n";
    std::cout << csv_header.str() << "\n" << csv_data.str() << "\n";
  }
}


void print_timings(timestamp start, timestamp parse_end, timestamp simulation_end) {
  std::cout << "----------\n" << std::setprecision(2);

  const auto total_time = std::chrono::duration<double>(simulation_end - start).count();
  std::cout << "Simulated 1 configuration in " << total_time << " s\n";

  const auto parse_time     = std::chrono::duration<double>(parse_end - start).count();
  const auto parse_time_pct = (parse_time / total_time) * 100;
  std::cout << "  Reading trace file took " << parse_time << " s (" << parse_time_pct
            << "%)\n";

  const auto sim_time = std::chrono::duration<double>(simulation_end - parse_end).count();
  const auto sim_time_pct = (sim_time / total_time) * 100;
  std::cout << "  Simulation took " << sim_time << " s (" << sim_time_pct << "%)\n";
}
