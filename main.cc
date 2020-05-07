#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: scs TRACE-FILE\n";
    return 1;
  }

  size_t count = 0;
  std::map<int, size_t> addresses;

  std::ifstream trace(argv[1]);
  for (std::string line; std::getline(trace, line);) {
    std::remove(line.begin(), line.end(), ',');
    // std::cout << "Line: " << line << "\n";

    std::istringstream iss(line);
    int seq, tid, size;
    bool is_bundle, is_write;
    long address, pc;
    iss >> seq >> tid >> is_bundle >> is_write >> size;
    iss >> std::hex >> address >> pc;
    // std::cout << "Values: " << seq << " " << tid << " " << is_bundle << " "
    // << is_write << " " << size << " " << address << " " << pc << "\n";

    count++;
    addresses[address]++;
  }

  std::cout << "Processed " << count << " lines.\n";
  std::cout << "Seen " << addresses.size() << " unique addresses.\n";

  return 0;
}
