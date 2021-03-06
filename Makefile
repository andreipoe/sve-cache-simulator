# ifeq ($(origin .RECIPEPREFIX), undefined)
#   $(error This Make does not support .RECIPEPREFIX. Please use GNU Make 4.0 or later)
# endif
# .RECIPEPREFIX = >

SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
.DELETE_ON_ERROR:

MAKEFLAGS += --warn-undefined-variables --no-builtin-rules

export

# -------

MACHINE = $(shell uname -m)
OS = $(shell uname -s)

# -------

COMPILER = CLANG
ifeq ($(COMPILER), INTEL)
ARCH = HOST
else
ARCH = native
endif

# The target CPU is specificed differently on x86 and on aarch64
# https://community.arm.com/developer/tools-software/tools/b/tools-software-ides-blog/posts/compiler-flags-across-architectures-march-mtune-and-mcpu
ifeq ($(MACHINE), aarch64)
ARCHFLAG = mcpu
else
ARCHFLAG = march
endif

CXX_ARM = armclang++
CXX_CLANG = clang++
CXX_CRAY = CC
CXX_GNU = g++
CXX_INTEL = icpc
CXX = $(CXX_$(COMPILER))

CXXFLAGS_ARM = -$(ARCHFLAG)=$(ARCH) -Ofast -ffp-contract=fast -fsimdmath -fopenmp
CXXFLAGS_CLANG = -$(ARCHFLAG)=$(ARCH) -Ofast -ffp-contract=fast -fopenmp
CXXFLAGS_CRAY = -fopenmp
CXXFLAGS_GNU = -$(ARCHFLAG)=$(ARCH) -Ofast -fopenmp
CXXFLAGS_INTEL = -Ofast -x$(ARCH) -qopenmp

CXXFLAGS = -Wall -Wextra -Wpedantic $(CXXFLAGS_$(COMPILER))
ifeq ($(DEBUG), 1)
CXXFLAGS += -g -O0
endif

LDFLAGS_ARM = -flto -fopenmp
LDFLAGS_CLANG = -flto -fopenmp
LDFLAGS_CRAY = -flto -fopenmp
LDFLAGS_GNU = -flto -fopenmp
LDFLAGS_INTEL = -ipo -qopenmp

ifeq ($(OS),Darwin)
LDFLAGS_CLANG += -L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib -mlinker-version=305
endif
LDFLAGS = $(LDFLAGS_$(COMPILER))

TARGET := scs
SRC := $(filter-out TraceConverterMain.cc BundleStatsMain.cc, $(wildcard *.cc))
OBJ := $(patsubst %.cc,%.o,$(SRC))
# HDR := $(patsubst %.cc,%.hh,$(SRC))

CONVERTER_TARGET := convert-trace
CONVERTER_SRC := MemoryTrace.cc TraceConverter.cc TraceConverterMain.cc
CONVERTER_OBJ := $(patsubst %.cc,%.o,$(CONVERTER_SRC))

BUNDLESTATS_TARGET := bundle-stats
BUNDLESTATS_SRC := BundleStatsMain.cc MemoryTrace.cc
BUNDLESTATS_OBJ := $(patsubst %.cc,%.o,$(BUNDLESTATS_SRC))

.PHONY: all converter bundlestats test clean

all: $(TARGET) converter bundlestats

converter: $(CONVERTER_TARGET)

bundlestats: $(BUNDLESTATS_TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

$(CONVERTER_TARGET): $(CONVERTER_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

$(BUNDLESTATS_TARGET): $(BUNDLESTATS_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

test: $(OBJ)
	$(MAKE) -C test


%.o: %.cc
	$(CXX) -std=c++17 $(CXXFLAGS) -c $^

clean:
	rm -f $(TARGET) *.o *.gch
	$(MAKE) -C test clean
