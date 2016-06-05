##########################################################################################
##                                                                                      ##
##                                 libgost15 benchmark                                  ##
##                                                                                      ##
##########################################################################################

cmake_minimum_required(VERSION 3.2)

## libgost15 project declaration
project(benchmark VERSION 0.3.6 LANGUAGES CXX)

## benchmark definition
add_executable(benchmark src/benchmark.cpp)

## Falling back to strict C++11 standard
set_target_properties(benchmark PROPERTIES CXX_STANDARD 11)
set_target_properties(benchmark PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(benchmark PROPERTIES CXX_EXTENSIONS OFF)

## Excluding benchmark from all target
set_target_properties(benchmark PROPERTIES EXCLUDE_FROM_ALL ON)

## Linking libgost15
target_link_libraries(benchmark libgost15)