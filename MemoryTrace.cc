#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <sstream>
#include <string>
#include <thread>

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


namespace MemoryTraceTools {

/* Guess if the given file is a text file */
TraceFileType guess_file_type(const std::string& fname) {
  std::ifstream f { fname };

  if (!f.is_open()) throw std::invalid_argument("Cannot open trace file: " + fname);

  size_t check_count { 500 };
  std::unique_ptr<char> data { new char[check_count + 1] };
  f.read(data.get(), check_count);

  if (f.eof()) check_count = f.gcount();
  if (std::memchr(data.get(), '\0', check_count) != NULL)
    return TraceFileType::Binary;
  else
    return TraceFileType::Text;
}
}  // namespace MemoryTraceTools


MemoryTrace::MemoryTrace(std::istream& tracefile, TraceFileType ftype) {
  switch (ftype) {
    case TraceFileType::Text:
      construct_from_text_(tracefile);
      break;
    case TraceFileType::Binary:
      construct_from_binary_serial_(tracefile);
      break;
    default:
      throw std::invalid_argument("Unknown trace file type");
  }
}

MemoryTrace::MemoryTrace(std::istream&& tracefile, TraceFileType ftype)
    : MemoryTrace(tracefile, ftype) { }

MemoryTrace::MemoryTrace(const std::string& trace_fname, TraceFileType ftype,
                         int io_threads) {
  switch (ftype) {
    case TraceFileType::Text:
      construct_from_text_(trace_fname);
      break;
    case TraceFileType::Binary:
      construct_from_binary_parallel_(trace_fname, io_threads);
      break;
    default:
      throw std::invalid_argument("Unknown trace file type");
  }
}

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

void MemoryTrace::construct_from_text_(const std::string& trace_fname) {
  std::ifstream tracefile { trace_fname };
  construct_from_text_(tracefile);
}

void MemoryTrace::construct_from_binary_serial_(std::istream& tracefile) {
  size_t elements;
  tracefile.read(reinterpret_cast<char*>(&elements), sizeof(size_t));

  requests.reserve(elements);
  requestAddresses.reserve(elements);

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

void MemoryTrace::construct_from_binary_parallel_(const std::string& trace_fname,
                                                  int io_threads) {

  std::ifstream tracefile { trace_fname, std::ios::binary };
  size_t elements;
  tracefile.read(reinterpret_cast<char*>(&elements), sizeof(size_t));
  tracefile.close();

  requests         = std::vector<MemoryRequest>(elements);
  requestAddresses = std::vector<uint64_t>(elements);

  const size_t max_threads =
      std::min(static_cast<unsigned>(io_threads), std::thread::hardware_concurrency());
  const auto nthreads              = std::min(elements, max_threads);
  const size_t elements_per_thread = std::ceil(elements / static_cast<double>(nthreads));
  const size_t bytes_per_thread =
      elements_per_thread * (3 * sizeof(int) + sizeof(bool) + 2 * sizeof(uint64_t));

  std::vector<std::thread> threads;
  threads.reserve(nthreads);

  for (size_t thread_num = 0; thread_num < nthreads; thread_num++) {
    threads.emplace_back([&, thread_num]() {
      std::ifstream tracefile { trace_fname, std::ios::binary };
      const size_t file_offset = sizeof(size_t) + bytes_per_thread * thread_num;

      tracefile.seekg(file_offset);

      for (size_t i = 0;
           i < elements_per_thread && i + elements_per_thread * thread_num < elements;
           i++) {
        const size_t global_element = elements_per_thread * thread_num + i;

        uint64_t address;

        tracefile.read(reinterpret_cast<char*>(&requests[global_element].tid),
                       sizeof(int));
        tracefile.read(reinterpret_cast<char*>(&requests[global_element].size),
                       sizeof(int));
        tracefile.read(reinterpret_cast<char*>(&requests[global_element].bundle_kind),
                       sizeof(int));
        tracefile.read(reinterpret_cast<char*>(&requests[global_element].is_write),
                       sizeof(bool));
        tracefile.read(reinterpret_cast<char*>(&address), sizeof(uint64_t));
        tracefile.read(reinterpret_cast<char*>(&requests[global_element].pc),
                       sizeof(uint64_t));

        requests[global_element].address = address;
        requestAddresses[global_element] = address;
      }
    });
  }

  for (auto& t : threads) t.join();
}


size_t MemoryTrace::getLength() const {
  assert(requests.size() == requestAddresses.size());
  return requests.size();
}


void MemoryTrace::write_binary(const std::string& fname) const {
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
