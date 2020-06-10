#include <unistd.h>
#include <cstring>
#include <iostream>

#include "TraceConverter.hh"

using namespace TraceConverter;

#define EXIT_INVALID_OPTION    1
#define EXIT_INVALID_ARGUMENTS 2
#define EXIT_INPUT_NOT_FOUND   3
#define EXIT_OUTPUT_EXISTS     4

void usage(int exit_code = -1);

int main(int argc, char* argv[]) {
  signed char opt;
  std::string out_fname;
  bool force { false };

  while ((opt = getopt(argc, argv, "fo:")) != -1) {
    switch (opt) {
      case 'f':
        force = true;
        break;
      case 'o':
        out_fname = optarg;
        break;
      case '?':
      default:
        usage(EXIT_INVALID_OPTION);
    }
  }
  argc -= optind;
  argv += optind;

  if (argc < 1 || (argc > 1 && !out_fname.empty())) usage(EXIT_INVALID_ARGUMENTS);

  if (!out_fname.empty()) {
    const std::string in_fname { argv[0] };

    convert(in_fname, out_fname, force);
  } else {
    for (int i = 0; i < argc; i++) {
      const std::string in_fname { argv[i] };
      const std::string out_fname = make_default_outname(in_fname);

      convert(in_fname, out_fname, force);
    }
  }

  return 0;
}

void usage(int exit_code) {
  std::cout << "Usage:\n";
  std::cout << "  convert-trace [-f] [-o OUTPUT] INPUT\n";
  std::cout << "  convert-trace [-f] INPUT...\n";
  std::exit(exit_code);
}
