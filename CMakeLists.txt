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
add_executable(bench bench/benchmark.cpp)

target_link_libraries(result_tests PRIVATE GTest::gtest_main GTest::gtest)
target_link_libraries(bench PRIVATE benchmark::benchmark)

enable_testing()
gtest_discover_tests(result_tests)

add_test(NAME ResultTests COMMAND result_tests)

add_custom_target(benchmark
    COMMAND $<TARGET_FILE:bench> --benchmark_counters_tabular=true
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS bench
)

if(DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM)
endif()
