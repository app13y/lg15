cmake_minimum_required(VERSION 3.2)

project(lg15
    VERSION 0.5.0
    LANGUAGES C CXX)

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
