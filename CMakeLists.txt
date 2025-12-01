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

# Warnings
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Library
add_library(particlesim STATIC
    src/particle_system.cpp
)

target_include_directories(particlesim PUBLIC include)

# Testing
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(particlesim_tests
    tests/test_particle_basics.cpp
)

target_link_libraries(particlesim_tests PRIVATE gtest_main particlesim)

include(GoogleTest)
gtest_discover_tests(particlesim_tests)