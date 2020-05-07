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

COMPILER = CLANG
ifeq ($(COMPILER), INTEL)
ARCH = HOST
else
ARCH = native
endif

CXX_ARM = armclang++
CXX_CLANG = clang++
CXX_CRAY = CC
CXX_GNU = g++
CXX_INTEL = icpc
CXX = $(CXX_$(COMPILER))

CXXFLAGS_ARM = -mcpu=$(ARCH) -Ofast -ffp-contract=fast -fsimdmath
CXXFLAGS_CLANG = -march=$(ARCH) -Ofast -ffp-contract=fast
CXXFLAGS_CRAY =
CXXFLAGS_GNU = -mcpu=$(ARCH) -Ofast
CXXFLAGS_INTEL = -Ofast -x$(ARCH)

CXXFLAGS = -Wall $(CXXFLAGS_$(COMPILER))

LDFLAGS_ARM = -flto
LDFLAGS_CLANG = -flto
LDFLAGS_CRAY = -flto
LDFLAGS_GNU = -flto
LDFLAGS_INTEL = -ipo

ifeq ($(shell uname -s),Darwin)
LDFLAGS_CLANG += -L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib -mlinker-version=305
endif
LDFLAGS = $(LDFLAGS_$(COMPILER))

TARGET := scs
SRC := $(wildcard *.cc)
OBJ := $(patsubst %.cc,%.o,$(SRC))
# OBJ := $(addsuffix .o,$(basename $(SRC)))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) -std=c++17 $(CXXFLAGS) -c $^

clean:
	rm -f $(TARGET) *.o
