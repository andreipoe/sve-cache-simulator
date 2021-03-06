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
#define EXIT_CONFIG_NOT_FOUND  6

#define OPT_ENCODING_TEXT   1
#define OPT_ENCODING_BINARY 2

#define OPT_DEFAULT_LIFETIMES_FNAME "lifetimes.csv"
#define OPT_DEFAULT_BUNDLES_FNAME   "bundles.csv"

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
  std::cout << "  scs --help\n";
  std::cout << "            \n";
  std::cout << "Options:\n";
  std::cout << "  -b, --batch BATCH-FILE        Treat all entries in BATCH-FILE as arguments to -c\n";
  std::cout << "                                Paths are relative to the batch file. May be specified more than once.\n";
  std::cout << "  -c CONFIG-FILE                The cache configuration file. Required at least once.\n";
  std::cout << "                                May be specified more than once for batch runs.\n";
  std::cout << "  -p, --io-threads N            Set the number of threads used for reading binary trace files.\n";
  std::cout << "                                Capped at `ncpus`. Default: min(" << DEFAULT_IO_THREADS << ", `ncpus`).\n";
  std::cout << "  -f, --format {text,csv,both}  Set the output format. Default: 'both' for single runs, 'csv' for batches.\n\n";
  std::cout << "  -t, --timings                 Report run times of the main stages.\n";
  std::cout << "                                \n";
  std::cout << "Additional Experiment Options:\n";
  std::cout << "  -d, --save-lifetimes          Save a CSV histogram of cache line lifetimes (\"evict distance\").\n";
  std::cout << "  -l, --save-bundles            Save a CSV list of SVE bundles encountered.\n";
  // clang-format on

  std::exit(code);
}

}  // namespace

std::string config_name_from_fname(std::string_view fname);
void print_text_results(const CacheHierarchy& cache, const MemoryTrace& trace,
                        std::string_view config_fname);
std::string make_csv_header();
std::string make_csv_results(const CacheHierarchy& cache, std::string_view config_name);
std::string make_csv_lifetimes_header();
std::string make_csv_lifetimes(const CacheHierarchy& cache,
                               const std::string& config_name);
std::string make_csv_bundles_header();
std::string make_csv_bundles(const CacheHierarchy& cache, const std::string& config_name);

using timestamp = std::chrono::time_point<std::chrono::high_resolution_clock>;
struct SimulationStats {
  std::string sim_name;
  std::shared_ptr<CacheHierarchy> cache;

  timestamp sim_start, sim_end;
  std::string csv_results, csv_lifetimes, csv_bundles;

  SimulationStats(const std::string& sim_name,
                  const std::shared_ptr<CacheHierarchy> cache)
      : sim_name(sim_name), cache(cache) { }

  explicit SimulationStats(const std::string& sim_name,
                           const std::shared_ptr<CacheHierarchy> cache,
                           const timestamp sim_start, const timestamp sim_end)
      : sim_name(sim_name), cache(cache), sim_start(sim_start), sim_end(sim_end) { }
};

void print_timings(timestamp start, timestamp parse_end,
                   const std::vector<SimulationStats>& simulations, timestamp finish);


int main(int argc, char* argv[]) {
  int opt, int_optarg;
  std::vector<std::string> config_fnames, batch_names;
  bool encoding_provided { false }, enable_timing { false }, opt_f_used { false },
      save_lifetimes { false }, save_bundles { false };
  int io_threads { DEFAULT_IO_THREADS };
  TraceFileType trace_encoding {};
  OutputFormat output_format { (1 << OUTPUT_BIT_COUNT) - 1 };

  struct option long_options[] = { { "config", required_argument, NULL, 'c' },
                                   { "batch", required_argument, NULL, 'b' },
                                   { "text", no_argument, NULL, OPT_ENCODING_TEXT },
                                   { "binary", no_argument, NULL, OPT_ENCODING_BINARY },
                                   { "io-threads", required_argument, NULL, 'p' },
                                   { "format", required_argument, NULL, 'f' },
                                   { "timings", no_argument, NULL, 't' },
                                   { "save-lifetimes", no_argument, NULL, 'd' },
                                   { "save-bundles", no_argument, NULL, 'l' },
                                   { "help", no_argument, NULL, 'h' },
                                   { 0, 0, 0, 0 } };

  while ((opt = getopt_long(argc, argv, "c:b:p:f:tdlh", long_options, NULL)) != -1) {
    switch (opt) {
      // Config options
      case 'c':
        config_fnames.emplace_back(optarg);
        break;
      case 'b':
        batch_names.emplace_back(optarg);
        break;

      // Encoding options
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

      // Tunables
      case 'p':
        int_optarg = std::stoi(optarg);
        if (int_optarg < 1) usage(EXIT_INVALID_ARGUMENTS);
        io_threads = int_optarg;
        break;

      // Output options
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
      case 'd':
        save_lifetimes = true;
        break;
      case 'l':
        save_bundles = true;
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

  const std::string trace_fname { argv[0] };
  if (!encoding_provided) try {
      trace_encoding = MemoryTraceTools::guess_file_type(argv[0]);
    } catch (const std::invalid_argument& e) {
      std::exit(EXIT_INVALID_TRACE);
    }

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
  if (!encoding_provided) info_output << " (guessed)";
  info_output << "\n";

  if (output_format[BIT_OUTPUT_TEXT]) std::cout << info_output.str();

  const timestamp t_start = std::chrono::high_resolution_clock::now();

  MemoryTrace trace { trace_fname, trace_encoding, io_threads };

  const timestamp t_parse_end = std::chrono::high_resolution_clock::now();

  if (output_format[BIT_OUTPUT_TEXT]) {
    const auto addresses = trace.getRequestAddresses();
    std::set<uint64_t> unique_addresses(addresses.begin(), addresses.end());
    std::cout << "Trace has " << trace.getLength() << " entries.\n";
    std::cout << "Seen " << unique_addresses.size() << " unique addresses.\n";
  }

  // Prepare a SmulationStats object to be populated as configurations are executed
  int max_levels = 0;
  std::vector<std::shared_ptr<CacheHierarchy>> caches;
  std::vector<SimulationStats> simulation_stats;
  caches.reserve(config_fnames.size());
  simulation_stats.reserve(config_fnames.size());
  for (const auto& config_fname : config_fnames) {
    std::ifstream config_file { config_fname };
    if (!config_file.is_open()) {
      std::cout << "Cannot open config file: " << config_fname << "\n";
      std::exit(EXIT_CONFIG_NOT_FOUND);
    }

    auto cache = std::make_shared<CacheHierarchy>(std::move(config_file));
    caches.push_back(cache);
    simulation_stats.emplace_back(config_name_from_fname(config_fname), cache);

    const int this_cache_levels = cache->nlevels();
    if (this_cache_levels > max_levels) max_levels = this_cache_levels;
  }


  // Main simulation loop
#pragma omp parallel for
  for (size_t i = 0; i < simulation_stats.size(); i++) {
    auto& sim = simulation_stats[i];

    sim.sim_start = std::chrono::high_resolution_clock::now();
    sim.cache->touch(trace.getRequests());
    sim.sim_end = std::chrono::high_resolution_clock::now();

    if (output_format[BIT_OUTPUT_TEXT])
      print_text_results(*sim.cache, trace, sim.sim_name);

    if (output_format[BIT_OUTPUT_CSV])
      sim.csv_results = make_csv_results(*sim.cache, sim.sim_name);

    if (save_lifetimes) sim.csv_lifetimes = make_csv_lifetimes(*sim.cache, sim.sim_name);
    if (save_bundles) sim.csv_bundles = make_csv_bundles(*sim.cache, sim.sim_name);
  }


  if (output_format[BIT_OUTPUT_CSV]) {
    if (output_format[BIT_OUTPUT_TEXT]) std::cout << SEPARATOR "\n";

    std::cout << make_csv_header() << "\n";
    for (const auto& sim : simulation_stats) std::cout << sim.csv_results;
  }

  // Additional experiments
  if (save_lifetimes) {
    std::ofstream f { OPT_DEFAULT_LIFETIMES_FNAME };
    f << make_csv_lifetimes_header() << "\n";
    for (const auto& sim : simulation_stats) f << sim.csv_lifetimes << "\n";
  }
  if (save_bundles) {
    std::ofstream f { OPT_DEFAULT_BUNDLES_FNAME };
    f << make_csv_bundles_header() << "\n";
    for (const auto& sim : simulation_stats) f << sim.csv_bundles << "\n";
  }

  const auto t_finish = std::chrono::high_resolution_clock::now();
  if (enable_timing) print_timings(t_start, t_parse_end, simulation_stats, t_finish);

  return 0;
}


/* Makes a config name from a given file name by taking the basename and removing the
 * extension */
std::string config_name_from_fname(std::string_view fname) {
  const auto last_slash  = fname.rfind('/');
  const auto last_dot    = fname.rfind('.');
  const auto config_name = fname.substr(last_slash + 1, last_dot - last_slash - 1);

  return std::string(config_name);
}


void print_text_results(const CacheHierarchy& cache, const MemoryTrace& trace,
                        std::string_view config_fname) {
  std::ostringstream ss;

  ss << SEPARATOR "\n";
  ss << "Config file: " << config_fname << "\n\n";
  ss << "CPU to L1 traffic: " << cache.getTraffic(0) << " bytes\n";

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

    ss << "\n";
    ss << level_names[level] << " Total accesses: " << total << "\n";
    ss << level_names[level] << " Hits: " << hits << " (" << std::fixed
       << std::setprecision(2) << pct_hits << "%)\n";
    ss << level_names[level] << " Misses: " << misses << " (" << std::fixed
       << std::setprecision(2) << pct_misses << "%)\n";
    ss << level_names[level] << " Evictions: " << evictions << "\n";
    ss << level_names[level] << " to " << level_names[level + 1]
       << " traffic: " << cache.getTraffic(level) << " bytes\n";
  }

  const auto bundles = cache.getBundleOps();
  uint64_t total_bundles { 0 }, total_bundle_ops { 0 };
  for (const auto b : bundles) {
    total_bundles += b.second.times_encountered;
    total_bundle_ops += b.second.total_ops;
  }
  const auto bundle_ratio =
      static_cast<double>(total_bundle_ops) / trace.getLength() * 100;

  ss << "\n";
  ss << "Total scatter/gather bundles simulated: " << total_bundles << "\n";
  ss << "Total unique scatter/gather bundles encountered: " << bundles.size() << "\n";
  ss << "Total ops part of scatters/gathers: " << total_bundle_ops << " ("
     << std::setprecision(2) << bundle_ratio << "%)\n";

  std::cout.imbue({ std::locale(), new CommaNumPunct() });
  std::cout << ss.str();
}

std::string make_csv_header() {
  return "config,level,accesses,misses,evictions,traffic-up";
}

std::string make_csv_results(const CacheHierarchy& cache, std::string_view config_name) {
  std::ostringstream csv;

  for (int level = 1; level <= cache.nlevels(); level++) {
    const auto total      = cache.getTotalAccesses(level);
    const auto misses     = cache.getMisses(level);
    const auto evictions  = cache.getEvictions(level);
    const auto traffic_up = cache.getTraffic(level);

    csv << config_name << ',' << level << ',' << total << ',' << misses << ','
        << evictions << ',' << traffic_up << '\n';
  }

  return csv.str();
}

void print_timings(timestamp start, timestamp parse_end,
                   const std::vector<SimulationStats>& simulation_stats,
                   timestamp finish) {
  std::cout << SEPARATOR "\n" << std::setprecision(2);

  const auto total_time = std::chrono::duration<double>(finish - start).count();
  std::cout << "Simulated " << simulation_stats.size() << " configurations in "
            << total_time << " s\n";

  const auto parse_time     = std::chrono::duration<double>(parse_end - start).count();
  const auto parse_time_pct = (parse_time / total_time) * 100;
  std::cout << "  Reading trace file took " << parse_time << " s (" << parse_time_pct
            << "%)\n";

  for (const auto& sim : simulation_stats) {
    const auto sim_time =
        std::chrono::duration<double>(sim.sim_end - sim.sim_start).count();
    const auto sim_time_pct = (sim_time / total_time) * 100;
    std::cout << "  Simulating " << sim.sim_name << " took " << sim_time << " s ("
              << sim_time_pct << "%)\n";
  }
}


// Additional experiments

std::string make_csv_lifetimes_header() { return "config,level,time,count"; }

std::string make_csv_lifetimes(const CacheHierarchy& cache,
                               const std::string& config_name) {
  std::ostringstream csv;

  for (int level = 1; level <= cache.nlevels(); level++) {
    const auto lifetimes = cache.getLifetimes(level);
    for (const auto& lifetime : *lifetimes) {
      csv << config_name << ',' << level << ',' << lifetime.first << ','
          << lifetime.second << '\n';
    }
  }

  return csv.str();
}

std::string make_csv_bundles_header() { return "config,pc,size,count"; }

std::string make_csv_bundles(const CacheHierarchy& cache,
                             const std::string& config_name) {
  std::ostringstream csv;

  const auto bundles = cache.getBundleOps();
  for (const auto& [pc, bundle] : bundles) {
    csv << config_name << ",0x" << std::hex << pc << std::dec << ',' << bundle.total_ops
        << ',' << bundle.times_encountered << '\n';
  }

  return csv.str();
}
