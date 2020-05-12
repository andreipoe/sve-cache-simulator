#pragma once

#include <fstream>
#include <vector>

struct MemoryRequest {
  int tid, size;
  bool is_bundle, is_write;
  long address, pc;

  explicit MemoryRequest(const int tid, const int size, const bool is_bundle,
                         const bool is_write, const long address, const long pc);
};

/* Represents an ArmIE memory trace */
class MemoryTrace {
  std::vector<MemoryRequest> requests;
  std::vector<long> requestAddresses;

 public:
  /* Construct a MemoryTrace object from a trace file */
  explicit MemoryTrace(std::istream& tracefile);

  const std::vector<MemoryRequest> getRequests() const;
  const std::vector<long> getRequestAddresses() const;
  const size_t getLength() const;
};
