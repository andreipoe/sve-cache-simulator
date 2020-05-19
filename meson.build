project('sve-cache-simulator', 'cpp', version: '0.0.1',
  default_options: ['cpp_std=c++17', 'buildtype=release', 'b_lto=true', 'warning_level=3'])

# ------- Build Options -------
cpp_args = []
link_args = []

build_type = get_option('buildtype')
if build_type == 'release'
  cpp_args = ['-ffast-math']

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
  endif
elif build_type == 'debug'
  cpp_args += ['-Og', '-g3']
  link_args += ['-g3']
endif

# Fix Homebew LLVM build on Mac
if build_machine.system() == 'darwin'
  link_args += ['-L/usr/local/opt/llvm/lib', '-,-rpath,/usr/local/opt/llvm/lib', -'mlinker-version=305']
endif


# ------- Main Binary -------
src_common = files([
  'cache.cc',
  'CacheConfig.cc',
  'DirectMappedCache.cc',
  'InfiniteCache.cc',
  'MemoryTrace.cc',
  'SetAssociativeCache.cc'
])
src_main = files('main.cc')
main_exe = executable('scs', src_common + src_main, cpp_args: cpp_args, link_args: link_args)


# ------- Tests -------
src_test = files([
  'test/CacheConfigTest.cc',
  'test/CacheTest.cc',
  'test/DirectMappedCacheTest.cc',
  'test/MemoryTraceTest.cc',
  'test/SetAssociativeCacheTest.cc',
  'test/test.cc',
  'test/utils.cc'
])
test_exe = executable('scs-test', src_test, objects: main_exe.extract_objects(src_common),
  cpp_args: cpp_args, link_args: link_args)
test('Cacth2 tests', test_exe)

# Copy the test data to the build directory (this will only happen once, when running `meson setup`)
# test_data_dir = '$MESON_BUILD_ROOT/test'
# run_command('mkdir', test_data_dir)
# run_command('cp', '-r', '$MESON_SOURCE_ROOT/test/traces', test_data_dir)
# run_command('cp', '-r', '$MESON_SOURCE_ROOT/configs', test_data_dir)