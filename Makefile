# ifeq ($(origin .RECIPEPREFIX), undefined)
#   $(error This Make does not support .RECIPEPREFIX. Please use GNU Make 4.0 or later)
# endif
# .RECIPEPREFIX = >

SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
.DELETE_ON_ERROR:

MAKEFLAGS += --warn-undefined-variables --no-builtin-rules

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

CXXFLAGS_ARM = -$(ARCHFLAG)=$(ARCH) -Ofast -ffp-contract=fast -fsimdmath
CXXFLAGS_CLANG = -$(ARCHFLAG)=$(ARCH) -Ofast -ffp-contract=fast
CXXFLAGS_CRAY =
CXXFLAGS_GNU = -$(ARCHFLAG)=$(ARCH) -Ofast
CXXFLAGS_INTEL = -Ofast -x$(ARCH)

CXXFLAGS = -Wall $(CXXFLAGS_$(COMPILER))

LDFLAGS_ARM = -flto
LDFLAGS_CLANG = -flto
LDFLAGS_CRAY = -flto
LDFLAGS_GNU = -flto
LDFLAGS_INTEL = -ipo

ifeq ($(OS),Darwin)
LDFLAGS_CLANG += -L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib -mlinker-version=305
endif
LDFLAGS = $(LDFLAGS_$(COMPILER))

TARGET := scs
SRC := $(wildcard *.cc)
OBJ := $(patsubst %.cc,%.o,$(SRC))
HDR := $(patsubst %.cc,%.hh,$(SRC))
# OBJ := $(addsuffix .o,$(basename $(SRC)))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) -std=c++17 $(CXXFLAGS) -c $^

clean:
	rm -f $(TARGET) *.o *.gch
