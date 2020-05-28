#include <algorithm>
#include <cassert>
#include <sstream>
#include <string>

#include "MemoryTrace.hh"

MemoryRequest::MemoryRequest(const int tid, const int size, const int bundle_value,
                             const bool is_write, const uint64_t address,
                             const uint64_t pc)
    : tid(tid),
      size(size),
      bundle_kind(parse_bundle_kind(bundle_value)),
      is_write(is_write),
      address(address),
      pc(pc) {}

MemoryRequest::MemoryRequest(const int tid, const int size, const BundleKind bundle_kind,
                             const bool is_write, const uint64_t address,
                             const uint64_t pc)
    : tid(tid),
      size(size),
      bundle_kind(bundle_kind),
      is_write(is_write),
      address(address),
      pc(pc) {}

BundleKind MemoryRequest::parse_bundle_kind(int bundle_value) {
  switch (bundle_value) {
    case 0:
      return BundleKind::None;
    case 1:
    case 3:
      return BundleKind::Start;
    case 2:
      return BundleKind::Middle;
    case 4:
    case 6:
      return BundleKind::Middle;
    default:
      throw std::invalid_argument(std::to_string(bundle_value) +
                                  " is not a bundle kind value");
  }
}


MemoryTrace::MemoryTrace(std::istream& tracefile) {
  for (std::string line; std::getline(tracefile, line);) {
    line.erase(std::remove(line.begin(), line.end(), ','), line.end());

#ifdef DEBUG
    std::cout << "Line: " << line << "\n";
#endif

    // TODO: Consider not constructing a stringstream for every line in the trace file by
    // moving this outside the loop
    std::istringstream iss(line);

    int seq, tid, size, bundle_kind;
    bool is_write;
    uint64_t address, pc;
    iss >> seq >> tid >> bundle_kind >> is_write >> size;
    iss >> std::hex >> address >> pc;

#ifdef DEBUG
    std::cout << "Values: " << seq << " " << tid << " " << bundle_kind << " " << is_write
              << " " << size << " " << address << " " << pc << "\n";
#endif

    requests.emplace_back(tid, size, bundle_kind, is_write, address, pc);
    requestAddresses.push_back(address);
  }
}

MemoryTrace::MemoryTrace(std::istream&& tracefile) : MemoryTrace(tracefile) {}

const std::vector<MemoryRequest> MemoryTrace::getRequests() const { return requests; }

const std::vector<uint64_t> MemoryTrace::getRequestAddresses() const {
  if (requests.size() != requestAddresses.size()) {
    assert(false && "This should never happen");

    /* requestAddresses.reserve(requests.size());
    std::transform(requests.begin(), requests.end(),
    std::back_inserter(requestAddresses),
                   [](MemoryRequest const& req) -> uint64_t { return req.address; }); */
  }

  return requestAddresses;
}

size_t MemoryTrace::getLength() const {
  assert(requests.size() == requestAddresses.size());
  return requests.size();
}
