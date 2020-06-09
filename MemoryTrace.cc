#include <algorithm>
#include <cassert>
#include <sstream>
#include <string>

#include "MemoryTrace.hh"

MemoryRequest::MemoryRequest(const int tid, const int size, const int bundle_kind,
                             const bool is_write, const uint64_t address,
                             const uint64_t pc)
    : tid(tid),
      size(size),
      bundle_kind(bundle_kind),
      is_write(is_write),
      address(address),
      pc(pc) { }

bool MemoryRequest::is_bundle() const { return bundle_kind != 0; }
bool MemoryRequest::is_bundle_start() const { return bundle_kind & 0x1; }
bool MemoryRequest::is_bundle_middle() const { return bundle_kind & 0x2; }
bool MemoryRequest::is_bundle_end() const { return bundle_kind & 0x4; }


MemoryTrace::MemoryTrace(std::istream& tracefile) {
  std::istringstream iss;

  for (std::string line; std::getline(tracefile, line);) {
    if (line.empty()) continue;

    line.erase(std::remove(line.begin(), line.end(), ','), line.end());

#ifdef DEBUG
    std::cout << "Line: " << line << "\n";
#endif

    iss.clear();
    iss.str(line);

    int seq, tid, size, bundle_kind;
    bool is_write;
    uint64_t address, pc;
    iss >> std::dec >> seq >> tid >> bundle_kind >> is_write >> size;
    iss >> std::hex >> address >> pc;

#ifdef DEBUG
    std::cout << "Values: " << seq << " " << tid << " " << bundle_kind << " " << is_write
              << " " << size << " " << address << " " << pc << "\n";
#endif

    requests.emplace_back(tid, size, bundle_kind, is_write, address, pc);
    requestAddresses.push_back(address);
  }
}

MemoryTrace::MemoryTrace(std::istream&& tracefile) : MemoryTrace(tracefile) { }

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
