project(
    'cpp_result',
    'cpp',
    default_options: ['cpp_std=c++17', 'warning_level=2', 'werror=true'],
)

gtest_dep = dependency('gtest', required: false)
gtest_main_dep = dependency('gtest_main', required: false)
gbench_dep = dependency('benchmark', required: false)

cpp_result_feature_all = get_option('result_feature_all')
cpp_result_feature_unwrap = get_option('result_feature_unwrap')
cpp_result_feature_map = get_option('result_feature_map')
cpp_result_feature_andor = get_option('result_feature_andor')
cpp_result_feature_inspect = get_option('result_feature_inspect')
cpp_result_feature_contains = get_option('result_feature_contains')
cpp_result_feature_flatten = get_option('result_feature_flatten')
cpp_result_feature_optional = get_option('result_feature_optional')

if cpp_result_feature_all
    cpp_result_feature_unwrap = true
    cpp_result_feature_map = true
    cpp_result_feature_andor = true
    cpp_result_feature_inspect = true
    cpp_result_feature_contains = true
    cpp_result_feature_flatten = true
    cpp_result_feature_optional = true
endif

cpp_result_defines = [
    '-DCPP_RESULT_FEATURE_ALL=' + (cpp_result_feature_all ? '1' : '0'),
    '-DCPP_RESULT_FEATURE_UNWRAP=' + (cpp_result_feature_unwrap ? '1' : '0'),
    '-DCPP_RESULT_FEATURE_MAP=' + (cpp_result_feature_map ? '1' : '0'),
    '-DCPP_RESULT_FEATURE_ANDOR=' + (cpp_result_feature_andor ? '1' : '0'),
    '-DCPP_RESULT_FEATURE_INSPECT=' + (cpp_result_feature_inspect ? '1' : '0'),
    '-DCPP_RESULT_FEATURE_CONTAINS=' + (cpp_result_feature_contains ? '1' : '0'),
    '-DCPP_RESULT_FEATURE_FLATTEN=' + (cpp_result_feature_flatten ? '1' : '0'),
    '-DCPP_RESULT_FEATURE_OPTIONAL=' + (cpp_result_feature_optional ? '1' : '0'),
]

add_project_arguments(cpp_result_defines, language: 'cpp')

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
    cpp_args: ['-O3'],
)
benchmark('exception_errorcode_result', bench)

bench_try_macros = executable(
    'bench_try_macros',
    'bench/bench_try_macros.cpp',
    include_directories: inc,
    dependencies: gbench_dep,
    install: false,
    cpp_args: ['-O3'],
)
benchmark('bench_try_macros', bench_try_macros)

doxygen = find_program(
    meson.global_source_root() / 'docs/meson-doxygen.sh',
    required: false,
)
if doxygen.found()
    run_target(
        'doc',
        command: [doxygen, meson.global_source_root() / 'Doxyfile'],
        env: ['WORKING_ROOT=' + meson.global_source_root()],
    )
endif
