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


MemoryTrace::MemoryTrace(std::istream& tracefile, TraceFileType ftype) {
  switch (ftype) {
    case TraceFileType::Text:
      construct_from_text_(tracefile);
      break;
    case TraceFileType::Binary:
      construct_from_binary_(tracefile);
      break;
    default:
      throw std::invalid_argument("Unknown trace file type");
  }
}

MemoryTrace::MemoryTrace(std::istream&& tracefile, TraceFileType ftype)
    : MemoryTrace(tracefile, ftype) { }

const std::vector<MemoryRequest> MemoryTrace::getRequests() const { return requests; }

const std::vector<uint64_t> MemoryTrace::getRequestAddresses() const {
  if (requests.size() != requestAddresses.size()) {
    assert(false && "This should never happen");
  }

  return requestAddresses;
}

void MemoryTrace::construct_from_text_(std::istream& tracefile) {
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

void MemoryTrace::construct_from_binary_(std::istream& tracefile) {
  size_t elements;
  tracefile.read(reinterpret_cast<char*>(&elements), sizeof(size_t));

  for (size_t i = 0; i < elements; i++) {
    int tid, size, bundle_kind;
    bool is_write;
    uint64_t address, pc;

    tracefile.read(reinterpret_cast<char*>(&tid), sizeof(int));
    tracefile.read(reinterpret_cast<char*>(&size), sizeof(int));
    tracefile.read(reinterpret_cast<char*>(&bundle_kind), sizeof(int));
    tracefile.read(reinterpret_cast<char*>(&is_write), sizeof(bool));
    tracefile.read(reinterpret_cast<char*>(&address), sizeof(uint64_t));
    tracefile.read(reinterpret_cast<char*>(&pc), sizeof(uint64_t));

    requests.emplace_back(tid, size, bundle_kind, is_write, address, pc);
    requestAddresses.push_back(address);
  }
}


size_t MemoryTrace::getLength() const {
  assert(requests.size() == requestAddresses.size());
  return requests.size();
}


void MemoryTrace::write_binary(const std::string fname) const {
  std::ofstream f { fname };

  const size_t length = getLength();
  f.write(reinterpret_cast<const char*>(&length), sizeof(size_t));

  for (const auto& request : requests) {
    f.write(reinterpret_cast<const char*>(&request.tid), sizeof(int));
    f.write(reinterpret_cast<const char*>(&request.size), sizeof(int));
    f.write(reinterpret_cast<const char*>(&request.bundle_kind), sizeof(int));
    f.write(reinterpret_cast<const char*>(&request.is_write), sizeof(bool));
    f.write(reinterpret_cast<const char*>(&request.address), sizeof(uint64_t));
    f.write(reinterpret_cast<const char*>(&request.pc), sizeof(uint64_t));
  }
}
