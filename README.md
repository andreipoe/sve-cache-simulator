# SVE Cache Simulator

[![pipeline status](https://gitlab.com/phd-repos/sve-cache-simulator/badges/master/pipeline.svg)](https://gitlab.com/phd-repos/sve-cache-simulator/-/commits/master)


## Build

```bash
make COMPILER=...
```

Compilers available: `ARM`, `CLANG` (default), `CRAY`, `GNU`, `INTEL`.

You can specify additional options by setting `ARCH`, `CXX_<compiler>`, `CXXFLAGS_<compiler>`, and `LDFLAGS_<compiler>`.

## Run

```bash
scs trace.log
```
