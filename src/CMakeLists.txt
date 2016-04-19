cmake_minimum_required(VERSION 3.2)

## gosthopper project declaration
project(gosthopper VERSION 0.3.6 LANGUAGES C)

## libgost15 library and selftests
add_subdirectory(libgost15)

## libgost15 benchmarks
add_subdirectory(benchmark)