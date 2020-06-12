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

#define SEPARATOR "----------"

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
  std::cout << "  -b, --batch BATCH-FILE        Treat all entries in BATCH-FILE as arguments to -c\n";
  std::cout << "                                Paths are relative to the batch file. May be specified more than once.\n";
  std::cout << "  -c CONFIG-FILE                The cache configuration file. Required at least once.\n";
  std::cout << "                                May be specified more than once for batch runs.\n";
  std::cout << "  -f, --format {text,csv,both}  Set the output format. Default: 'both' for single runs, 'csv' for batches.\n\n";
  std::cout << "  -t, --timings                 Report run times of the main stages.\n";
  // clang-format on

  std::exit(code);
}

}  // namespace

TraceFileType guess_file_type(const std::string& fname);
std::string_view config_name_from_fname(std::string_view fname);
void run_and_print_stats(MemoryTrace const& trace, CacheHierarchy& cache,
                         const OutputFormat& fmt, std::string_view config_fname);

using timestamp = std::chrono::time_point<std::chrono::high_resolution_clock>;
struct SimulationTime {
  std::string_view sim_name;
  timestamp sim_start, sim_end;

  explicit SimulationTime(std::string_view sim_name, const timestamp sim_start,
                          const timestamp sim_end)
      : sim_name(sim_name), sim_start(sim_start), sim_end(sim_end) { }
};

void print_timings(timestamp start, timestamp parse_end,
                   const std::vector<SimulationTime> simulations);


int main(int argc, char* argv[]) {
  int opt;
  std::vector<std::string> config_fnames, batch_names;
  bool encoding_provided { false }, enable_timing { false }, opt_f_used { false };
  TraceFileType trace_encoding {};
  OutputFormat output_format { (1 << OUTPUT_BIT_COUNT) - 1 };

  struct option long_options[] = { { "config", required_argument, NULL, 'c' },
                                   { "batch", required_argument, NULL, 'b' },
                                   { "text", no_argument, NULL, OPT_ENCODING_TEXT },
                                   { "binary", no_argument, NULL, OPT_ENCODING_BINARY },
                                   { "format", required_argument, NULL, 'f' },
                                   { "timings", no_argument, NULL, 't' },
                                   { "help", no_argument, NULL, 'h' },
                                   { 0, 0, 0, 0 } };

  while ((opt = getopt_long(argc, argv, "c:b:f:th", long_options, NULL)) != -1) {
    switch (opt) {
      case 'c':
        config_fnames.emplace_back(optarg);
        break;
      case 'b':
        batch_names.emplace_back(optarg);
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
        opt_f_used = true;
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

  for (const auto& batch : batch_names) {
    std::ifstream f { batch };
    std::string line;
    while (std::getline(f, line)) {
      if (line.empty()) continue;

      // Accept absolute and relative paths
      if (line[0] == '/')
        config_fnames.push_back(line);
      else {
        // Relative paths are relative to the the batch file's location
        const auto last_slash = batch.rfind('/');
        if (last_slash == std::string::npos)
          config_fnames.push_back(line);
        else
          config_fnames.push_back(batch.substr(0, last_slash) + '/' + line);
      }
    }
  }
  if (config_fnames.empty() || argc < 1) usage(EXIT_INVALID_ARGUMENTS);

  // In batch mode, if text output hasn't been request specifically, use CSV output by
  // default
  if (config_fnames.size() > 1 && !opt_f_used) output_format.reset(BIT_OUTPUT_TEXT);

  std::ostringstream info_output;

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

  if (output_format[BIT_OUTPUT_TEXT]) {
    const auto addresses = trace.getRequestAddresses();
    std::set<uint64_t> unique_addresses(addresses.begin(), addresses.end());
    std::cout << "Trace has " << trace.getLength() << " entries.\n";
    std::cout << "Seen " << unique_addresses.size() << " unique addresses.\n";
  }

  // TODO: When running a batch, each config can be run in a separate thread
  std::vector<std::string_view> configs_not_found;
  std::vector<SimulationTime> simulation_times;
  auto t_previous = std::chrono::high_resolution_clock::now();
  for (const auto& config_fname : config_fnames) {
    std::ifstream config_file { config_fname };
    if (!config_file.is_open()) {
      std::cout << "Cannot open config file: " << config_fname << "\n";
      configs_not_found.push_back(config_fname);
      continue;
    }

    CacheHierarchy cache { std::move(config_file) };
    run_and_print_stats(trace, cache, output_format, config_fname);

    const auto t_sim_end = std::chrono::high_resolution_clock::now();
    simulation_times.emplace_back(config_name_from_fname(config_fname), t_previous,
                                  t_sim_end);
    t_previous = t_sim_end;
  }

  if (enable_timing) print_timings(t_start, t_parse_end, simulation_times);

  if (configs_not_found.size() > 0) {
    std::cerr << "Config files not found: ";
    for (auto f : configs_not_found) std::cerr << f << " ";
    std::cerr << "\n";
    std::exit(EXIT_INVALID_CONFIG);
  }

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

/* Makes a config name from a given file name by taking the basename and removing the
 * extension */
std::string_view config_name_from_fname(std::string_view fname) {
  const auto last_slash  = fname.rfind('/');
  const auto last_dot    = fname.rfind('.');
  const auto config_name = fname.substr(last_slash + 1, last_dot - last_slash - 1);

  return config_name;
}


void run_and_print_stats(MemoryTrace const& trace, CacheHierarchy& cache,
                         const OutputFormat& fmt, std::string_view config_fname) {
  cache.touch(trace.getRequests());

  // TODO: Output needs to be kept in order if running simulations in parallel
  std::ostringstream csv_data, csv_header;

  if (fmt[BIT_OUTPUT_TEXT]) {
    std::cout << SEPARATOR "\n";

    std::cout.imbue({ std::locale(), new CommaNumPunct() });
    std::cout << "Config file: " << config_fname << "\n\n";

    std::cout << "CPU to L1 traffic: " << cache.getTraffic(0) << " bytes\n";
  }

  // The config name is the base file name without the extension

  csv_header << "config,CPU-L1-traffic,";
  csv_data << config_name_from_fname(config_fname) << ',' << cache.getTraffic(0) << ',';

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
      static_cast<double>(total_bundle_ops) / trace.getLength() * 100;

  if (fmt[BIT_OUTPUT_TEXT]) {
    std::cout << "\n";
    std::cout << "Total scatter/gather bundles simulated: " << total_bundles << "\n";
    std::cout << "Total unique scatter/gather bundles encountered: " << bundles.size()
              << "\n";
    std::cout << "Total ops part of scatters/gathers: " << total_bundle_ops << " ("
              << std::setprecision(2) << bundle_ratio << "%)\n";
  }

  // TODO: In batch mode, output the header only once, keeping in mind that it might
  // differ per configuration
  if (fmt[BIT_OUTPUT_CSV]) {
    if (fmt[BIT_OUTPUT_TEXT]) std::cout << SEPARATOR "\n";
    std::cout << csv_header.str() << "\n" << csv_data.str() << "\n";
  }
}


void print_timings(timestamp start, timestamp parse_end,
                   const std::vector<SimulationTime> simulation_times) {
  std::cout << SEPARATOR "\n" << std::setprecision(2);

  const auto t_finish   = simulation_times[simulation_times.size() - 1].sim_end;
  const auto total_time = std::chrono::duration<double>(t_finish - start).count();
  std::cout << "Simulated " << simulation_times.size() << " configurations in "
            << total_time << " s\n";

  const auto parse_time     = std::chrono::duration<double>(parse_end - start).count();
  const auto parse_time_pct = (parse_time / total_time) * 100;
  std::cout << "  Reading trace file took " << parse_time << " s (" << parse_time_pct
            << "%)\n";

  for (const auto& sim : simulation_times) {
    const auto sim_time =
        std::chrono::duration<double>(sim.sim_end - sim.sim_start).count();
    const auto sim_time_pct = (sim_time / total_time) * 100;
    std::cout << "  Simulating " << sim.sim_name << " took " << sim_time << " s ("
              << sim_time_pct << "%)\n";
  }
}
