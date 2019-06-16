cmake_minimum_required(VERSION 3.2)

set(IMPLEMENTATION "Universal"
    CACHE STRING "Implementations: Universal, SSE2")

add_library(lg15)

target_include_directories(lg15 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_sources(lg15 PRIVATE
    src/data.c
    src/universal.c)

target_compile_features(lg15 PUBLIC
    c_std_99
    cxx_std_11)
