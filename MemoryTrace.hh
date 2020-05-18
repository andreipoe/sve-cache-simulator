#pragma once

#include <fstream>
#include <vector>

struct MemoryRequest {
  int tid, size;
  bool is_bundle, is_write;
  uint64_t address, pc;

  explicit MemoryRequest(const int tid, const int size, const bool is_bundle,
                         const bool is_write, const uint64_t address, const uint64_t pc);
};

/* Represents an ArmIE memory trace */
class MemoryTrace {
  std::vector<MemoryRequest> requests;
  std::vector<uint64_t> requestAddresses;

 public:
  /* Construct a MemoryTrace object from a trace file */
  explicit MemoryTrace(std::istream& tracefile);

  const std::vector<MemoryRequest> getRequests() const;
  const std::vector<uint64_t> getRequestAddresses() const;
  size_t getLength() const;
};
