#pragma once

#include <fstream>
#include <vector>

struct MemoryRequest {
  int tid, size, bundle_kind;
  bool is_write;
  uint64_t address, pc;

  MemoryRequest() = default;
  explicit MemoryRequest(const int tid, const int size, const int bundle_value,
                         const bool is_write, const uint64_t address, const uint64_t pc);

  bool is_bundle() const;
  bool is_bundle_start() const;
  bool is_bundle_middle() const;
  bool is_bundle_end() const;

  friend std::ostream& operator<<(std::ostream& stream, const MemoryRequest& req) {
    stream << "Request{"
           << "tid: " << req.tid << ", bundle_kind: " << req.bundle_kind
           << ", is_write: " << req.is_write << ", size: " << req.size << ", address: 0x"
           << std::hex << req.address << ", pc: 0x" << req.pc << "}" << std::dec;
    return stream;
  }
};


enum class TraceFileType { Text, Binary };

/* Represents an ArmIE memory trace */
class MemoryTrace {
  std::vector<MemoryRequest> requests;
  std::vector<uint64_t> requestAddresses;

  inline void construct_from_text_(std::istream& tracefile);
  inline void construct_from_text_(const std::string& trace_fname);
  inline void construct_from_binary_serial_(std::istream& tracefile);
  inline void construct_from_binary_parallel_(const std::string& trace_fname);

 public:
  /* Construct a MemoryTrace object from a trace file.
     Binary traces are read sequentially */
  explicit MemoryTrace(std::istream& tracefile,
                       TraceFileType ftype = TraceFileType::Text);
  explicit MemoryTrace(std::istream&& tracefile,
                       TraceFileType ftype = TraceFileType::Text);

  /* Construct a MemoryTrace object from a trace file name
     Binary traces are read in parallel */
  explicit MemoryTrace(const std::string& trace_fname,
                       TraceFileType ftype = TraceFileType::Text);

  const std::vector<MemoryRequest> getRequests() const;
  const std::vector<uint64_t> getRequestAddresses() const;
  size_t getLength() const;

  /* Save this trace to a binary file */
  void write_binary(const std::string& fname) const;
};
