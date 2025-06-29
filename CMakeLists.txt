cmake_minimum_required(VERSION 3.12)
project(cpp_result LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(CPP_RESULT_FEATURE_ALL "Enable all cpp_result features" ON)
option(CPP_RESULT_FEATURE_UNWRAP "Enable unwrap group" OFF)
option(CPP_RESULT_FEATURE_MAP "Enable map group" OFF)
option(CPP_RESULT_FEATURE_ANDOR "Enable and/or group" OFF)
option(CPP_RESULT_FEATURE_INSPECT "Enable inspect group" OFF)
option(CPP_RESULT_FEATURE_CONTAINS "Enable contains group" OFF)
option(CPP_RESULT_FEATURE_FLATTEN "Enable flatten group" OFF)
option(CPP_RESULT_FEATURE_OPTIONAL "Enable optional group" OFF)

if(CPP_RESULT_FEATURE_ALL)
    set(CPP_RESULT_FEATURE_UNWRAP ON CACHE BOOL "" FORCE)
    set(CPP_RESULT_FEATURE_MAP ON CACHE BOOL "" FORCE)
    set(CPP_RESULT_FEATURE_ANDOR ON CACHE BOOL "" FORCE)
    set(CPP_RESULT_FEATURE_INSPECT ON CACHE BOOL "" FORCE)
    set(CPP_RESULT_FEATURE_CONTAINS ON CACHE BOOL "" FORCE)
    set(CPP_RESULT_FEATURE_FLATTEN ON CACHE BOOL "" FORCE)
    set(CPP_RESULT_FEATURE_OPTIONAL ON CACHE BOOL "" FORCE)
endif()

add_compile_definitions(
    CPP_RESULT_FEATURE_ALL=$<BOOL:${CPP_RESULT_FEATURE_ALL}>
    CPP_RESULT_FEATURE_UNWRAP=$<BOOL:${CPP_RESULT_FEATURE_UNWRAP}>
    CPP_RESULT_FEATURE_MAP=$<BOOL:${CPP_RESULT_FEATURE_MAP}>
    CPP_RESULT_FEATURE_ANDOR=$<BOOL:${CPP_RESULT_FEATURE_ANDOR}>
    CPP_RESULT_FEATURE_INSPECT=$<BOOL:${CPP_RESULT_FEATURE_INSPECT}>
    CPP_RESULT_FEATURE_CONTAINS=$<BOOL:${CPP_RESULT_FEATURE_CONTAINS}>
    CPP_RESULT_FEATURE_FLATTEN=$<BOOL:${CPP_RESULT_FEATURE_FLATTEN}>
    CPP_RESULT_FEATURE_OPTIONAL=$<BOOL:${CPP_RESULT_FEATURE_OPTIONAL}>
)

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
