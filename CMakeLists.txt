cmake_minimum_required(VERSION 3.2)

project(libgost15-bundle VERSION 0.4.0)

option(WITH_TESTS "Builds tests" OFF)
option(WITH_BENCHMARKS "Builds benchmarks" OFF)

add_subdirectory(libgost15)

if(WITH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(WITH_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
