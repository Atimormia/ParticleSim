cmake_minimum_required(VERSION 3.20)
project(ParticleSim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;Profile" CACHE STRING "" FORCE)

if (CMAKE_BUILD_TYPE STREQUAL "Profile")
    set(CMAKE_CXX_FLAGS_PROFILE "-O2 -g -fno-omit-frame-pointer" CACHE STRING "" FORCE)
endif()

# Options
option(PARTICLESIM_ENABLE_PROFILING "Enable profiling support" OFF)
option(PARTICLESIM_USE_SIMD "Enable SIMD optimizations" OFF)
option(PARTICLESIM_ENABLE_ASSERTS "Enable runtime asserts" ON)

option(ENABLE_BENCHMARKS "Build benchmarks" ON)
option(ENABLE_TESTS "Build unit tests" ON)

# Warnings
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

include(FetchContent)

# --------------------------
# Google Benchmark
# --------------------------
if (ENABLE_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    set(BENCHMARK_ENABLE_TESTING OFF)
    FetchContent_MakeAvailable(benchmark)
endif()

# --------------------------
# Tracy profiler
# --------------------------
#if (ENABLE_TRACY)
#    FetchContent_Declare(
#        tracy
#        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
#        GIT_TAG master
#    )
#    FetchContent_MakeAvailable(tracy)
#endif()

# Library
add_library(particlesim STATIC
    src/particle.cpp
    src/particle_system.cpp
    src/spatial_partitioning.cpp
)

target_include_directories(particlesim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(benchmarks)

# --------------------------
# Tests
# --------------------------
if (ENABLE_TESTS)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(particlesim_tests
        tests/test_spatial_partitioning.cpp
        tests/test_particle_system.cpp
        tests/test_particle.cpp
        tests/core/test_vector2d.cpp
        tests/core/test_allocator.cpp
        tests/test_helpers.cpp
    )
    
    target_compile_definitions(particlesim_tests PRIVATE ENABLE_TEST_METHODS)
    target_link_libraries(particlesim_tests PRIVATE gtest_main particlesim)

    include(GoogleTest)
    gtest_discover_tests(particlesim_tests)

    if(ENABLE_COVERAGE)
    target_compile_options(your_test_target PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(your_test_target PRIVATE -fprofile-instr-generate)
endif()

endif()

# Console
add_executable(particlesim_example examples/console.cpp)
target_link_libraries(particlesim_example PRIVATE particlesim)
