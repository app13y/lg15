cmake_minimum_required(VERSION 3.2)

add_executable(benchmark
    src/benchmark.cpp)

target_link_libraries(benchmark
    lg15)
