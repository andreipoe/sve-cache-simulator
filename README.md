# SVE Cache Simulator

## Build

```bash
make COMPILER=...
```

Compilers available: `ARM`, `CLANG` (default), `CRAY`, `GNU`, `INTEL`.

You can specify additional options by setting `ARCH`, `CXX_<compiler>`, `CXXFLAGS_<compiler>`, and `LDFLAGS_<compiler>`.

## Run

```bash
scs trace.log
```
