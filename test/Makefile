# This Makefile depends on the top-level Makefile
# Don't run `make` in this directory, but run `make test` in the root directory (above)

TEST := scs-test
SRC := $(filter-out ../main.cc ../TraceConverterMain.cc ../BundleStatsMain.cc, $(wildcard *.cc ../*.cc))
OBJ := $(patsubst %.cc,%.o,$(SRC))

.PHONY: all clean

all: $(TEST)

$(TEST): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) -I.. -std=c++17 $(CXXFLAGS) -c $^

clean:
	rm -f $(TEST) *.o *.gch

