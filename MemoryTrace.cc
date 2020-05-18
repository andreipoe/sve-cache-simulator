#include <algorithm>
#include <cassert>
#include <sstream>
#include <string>

#include "MemoryTrace.hh"

MemoryRequest::MemoryRequest(const int tid, const int size, const bool is_bundle,
                             const bool is_write, const uint64_t address, const uint64_t pc)
    : tid(tid),
      size(size),
      is_bundle(is_bundle),
      is_write(is_write),
      address(address),
      pc(pc) {}

MemoryTrace::MemoryTrace(std::istream& tracefile) {
  for (std::string line; std::getline(tracefile, line);) {
    std::remove(line.begin(), line.end(), ',');

#ifdef DEBUG
    std::cout << "Line: " << line << "\n";
#endif

    std::istringstream iss(line);

    int seq, tid, size;
    bool is_bundle, is_write;
    uint64_t address, pc;
    iss >> seq >> tid >> is_bundle >> is_write >> size;
    iss >> std::hex >> address >> pc;

#ifdef DEBUG
    std::cout << "Values: " << seq << " " << tid << " " << is_bundle << " " << is_write
              << " " << size << " " << address << " " << pc << "\n";
#endif

    requests.emplace_back(tid, size, is_bundle, is_write, address, pc);
    requestAddresses.push_back(address);
  }
}

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
