#include <cassert>
#include <iostream>
#include <map>

#include "MemoryTrace.hh"

#define EXIT_INVALID_ARGUMENTS 1
#define EXIT_INVALID_TRACE     2

namespace {
struct BundleStats {
  int num_components { 1 };  // the number of contiguous chunks
  uint64_t address_delta;    // the difference between the highest and the lowest address
};

// Required to use the struct as a map key
bool operator<(const BundleStats& lhs, const BundleStats& rhs) {
  return lhs.num_components < rhs.num_components ||
         (lhs.num_components == rhs.num_components &&
          lhs.address_delta < rhs.address_delta);
}

}  // namespace

int main(int argc, char* argv[]) {

  if (argc < 2) {
    std::cout << "Usage: bundle-stats TRACE-FILE\n";
    std::exit(EXIT_INVALID_ARGUMENTS);
  }

  const std::string trace_fname { argv[1] };
  TraceFileType encoding;
  try {
    encoding = MemoryTraceTools::guess_file_type(trace_fname);
  } catch (const std::invalid_argument& e) {
    std::exit(EXIT_INVALID_TRACE);
  }
  MemoryTrace trace { trace_fname, encoding };

  std::map<BundleStats, uint64_t> bundles;

  const auto requests = trace.getRequests();
  for (size_t i = 0; i < requests.size(); i++) {
    const auto& bundle_start = requests[i];
    if (!bundle_start.is_bundle() || bundle_start.bundle_kind == 7) continue;

    assert(bundle_start.is_bundle_start() &&
           "First request in a bundle not a bundle start");

    BundleStats this_bundle;
    do {
      i++;
      this_bundle.num_components++;
    } while (!requests[i].is_bundle_end());

    const auto& bundle_end = requests[i];
    const auto range_start = std::min(bundle_start.address, bundle_end.address);
    const auto range_end = std::max(bundle_start.address, bundle_end.address);
    this_bundle.address_delta =
        range_end - range_start + bundle_end.size;

    bundles[this_bundle]++;
  }

  std::cout << "components,delta,count\n";
  for (const auto& [bundle, count] : bundles)
    std::cout << bundle.num_components << ',' << bundle.address_delta << ',' << count
              << '\n';

  return 0;
}
