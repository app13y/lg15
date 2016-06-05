cmake_minimum_required(VERSION 3.2)

## libgost15-lib project declaration
project(libgost15-wrap VERSION 0.4.0)

## libgost15 library and selftests
add_subdirectory(libgost15)

## libgost15 benchmarks
add_subdirectory(benchmark)