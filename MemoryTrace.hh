#pragma once

#include <fstream>
#include <vector>

enum class BundleKind { None, Start, Middle, End };

struct MemoryRequest {
  int tid, size;
  BundleKind bundle_kind;
  bool is_write;
  uint64_t address, pc;

  explicit MemoryRequest(const int tid, const int size, const int bundle_value,
                         const bool is_write, const uint64_t address, const uint64_t pc);
  explicit MemoryRequest(const int tid, const int size, const BundleKind bundle_kind,
                         const bool is_write, const uint64_t address, const uint64_t pc);

  static BundleKind parse_bundle_kind(int value);
};

/* Represents an ArmIE memory trace */
class MemoryTrace {
  std::vector<MemoryRequest> requests;
  std::vector<uint64_t> requestAddresses;

 public:
  /* Construct a MemoryTrace object from a trace file */
  explicit MemoryTrace(std::istream& tracefile);
  explicit MemoryTrace(std::istream&& tracefile);

  const std::vector<MemoryRequest> getRequests() const;
  const std::vector<uint64_t> getRequestAddresses() const;
  size_t getLength() const;
};
