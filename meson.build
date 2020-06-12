project('sve-cache-simulator', 'cpp', version: '0.0.1',
  default_options: ['cpp_std=c++17', 'buildtype=release', 'b_lto=true', 'warning_level=3'])

# ------- Build Options -------
cpp_args = []
link_args = []
rpath = ''

build_type = get_option('buildtype')
if build_type == 'release'
  cpp_args += ['-ffast-math']

  if get_option('parallel')
    cpp_args  += ['-fopenmp']
    link_args += ['-fopenmp']
  endif

  arch = get_option('arch')
  if target_machine.cpu_family() == 'aarch64'
    cpp_args += ['-mcpu=' + arch]
  else
    cpp_args += ['-march=' + arch]
  endif

  compiler = meson.get_compiler('cpp').get_id()
  if compiler == 'armclang'
    cpp_args += ['-ffp-contract=fast', '-fsimdmath']
  elif compiler == 'clang'
    cpp_args += ['-ffp-contract=fast']

    # Fix Homebew LLVM build on Mac
    if build_machine.system() == 'darwin'
      cpp_args  += ['-I/usr/local/opt/llvm/include']
      link_args += ['-L/usr/local/opt/llvm/lib']
      rpath = '/usr/local/opt/llvm/lib'
    endif
  endif
elif build_type == 'debug' and meson.get_compiler('cpp').get_id() == 'gcc'
  cpp_args  += ['-g3']
  link_args += ['-g3']
endif


# ------- Main Binary -------
src_common = files([
  'cache.cc',
  'CacheConfig.cc',
  'CacheHierarchy.cc',
  'DirectMappedCache.cc',
  'InfiniteCache.cc',
  'MemoryTrace.cc',
  'SetAssociativeCache.cc'
])
src_main = files('main.cc')
main_exe = executable('scs', src_common, src_main,
    cpp_args: cpp_args,
    link_args: link_args,
    build_rpath: rpath)


# ------- Trace Converter -------
src_converter_common = files('TraceConverter.cc')
src_converter_main = files([
  'MemoryTrace.cc',
  'TraceConverterMain.cc'])
converter_exe = executable('convert-trace', src_converter_common, src_converter_main,
    cpp_args: cpp_args,
    link_args: link_args,
    build_rpath: rpath)


# ------- Tests -------
src_test = files([
  'test/CacheConfigTest.cc',
  'test/CacheTest.cc',
  'test/CacheHierarchyTest.cc',
  'test/DirectMappedCacheTest.cc',
  'test/InfiniteCacheTest.cc',
  'test/MemoryTraceTest.cc',
  'test/SetAssociativeCacheTest.cc',
  'test/RandomAddressGenerator.cc',
  'test/TraceConverterTest.cc',
  'test/test.cc',
  'test/utils.cc'
])
test_exe = executable('scs-test', src_test,
                      objects: [main_exe.extract_objects(src_common), converter_exe.extract_objects(src_converter_common)],
                      cpp_args: cpp_args,
                      link_args: link_args,
                      build_rpath: rpath)

test('Cacth2 tests', test_exe)

# We cannot reliably copy test trace files at build time
# See https://github.com/mesonbuild/meson/issues/860
