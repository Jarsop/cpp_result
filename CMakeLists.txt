cmake_minimum_required(VERSION 3.12)
project(cpp_result LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

find_package(GTest)
include(GoogleTest)
find_package(benchmark)
find_package(Doxygen)

add_executable(usage examples/usage.cpp)
add_executable(result_tests tests/result_tests.cpp)
add_executable(bench_exc_errorcode_result bench/benchmark.cpp)
add_executable(bench_try_macros bench/bench_try_macros.cpp)

# Force -O3 for benchmarks
set_target_properties(bench_exc_errorcode_result PROPERTIES COMPILE_OPTIONS "-O3")
set_target_properties(bench_try_macros PROPERTIES COMPILE_OPTIONS "-O3")

target_link_libraries(result_tests PRIVATE GTest::gtest_main GTest::gtest)
target_link_libraries(bench_exc_errorcode_result PRIVATE benchmark::benchmark)
target_link_libraries(bench_try_macros PRIVATE benchmark::benchmark)

enable_testing()
gtest_discover_tests(result_tests)

add_test(NAME ResultTests COMMAND result_tests)

add_custom_target(benchmark
    COMMAND $<TARGET_FILE:bench_exc_errorcode_result> --benchmark_counters_tabular=true
    COMMAND $<TARGET_FILE:bench_try_macros> --benchmark_counters_tabular=true
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS bench_exc_errorcode_result bench_try_macros
)

if(DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM)
endif()
