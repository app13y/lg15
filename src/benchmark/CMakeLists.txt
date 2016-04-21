##########################################################################################
##                                                                                      ##
##                                 libgost15 benchmark                                  ##
##                                                                                      ##
##########################################################################################

cmake_minimum_required(VERSION 3.2)

## libgost15 project declaration
project(libgost15-benchmark VERSION 0.3.5 LANGUAGES CXX)

add_executable(benchmark src/benchmark.cpp)
set_target_properties(benchmark PROPERTIES CXX_STANDARD 11)
target_link_libraries(benchmark libgost15)