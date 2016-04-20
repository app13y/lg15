cmake_minimum_required(VERSION 3.2)

## libgost15-lib project declaration
project(aprelev-libgost15 VERSION 0.3.6)

## libgost15 library and selftests
add_subdirectory(libgost15)

## libgost15 benchmarks
add_subdirectory(benchmark)