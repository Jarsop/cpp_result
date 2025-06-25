project(
    'cpp_result',
    'cpp',
    default_options: ['cpp_std=c++17', 'warning_level=3', 'werror=true'],
)

gtest_dep = dependency('gtest', required: false)
gtest_main_dep = dependency('gtest_main', required: false)
gbench_dep = dependency('benchmark', required: false)

inc = include_directories('include')

executable('usage', 'examples/usage.cpp', include_directories: inc)

test_exe = executable(
    'result_tests',
    'tests/result_tests.cpp',
    include_directories: inc,
    dependencies: [gtest_dep, gtest_main_dep],
)

test('ResultTests', test_exe)

bench = executable(
    'bench',
    'bench/benchmark.cpp',
    include_directories: inc,
    dependencies: gbench_dep,
)
benchmark('exception_errorcode_result', bench)
