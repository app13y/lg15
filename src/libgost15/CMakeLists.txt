##########################################################################################
##                                                                                      ##
##                                  libgost15 library                                   ##
##                                                                                      ##
##########################################################################################

cmake_minimum_required(VERSION 3.2)

## libgost15 project declaration
project(libgost15 VERSION 0.4.0 LANGUAGES C)

## libgost15 global options
set(IMPLEMENTATION "Optimised" CACHE STRING "Implementation version: Compact, Optimised or SIMD.")

## libgost15 definition
add_library(libgost15)

## libgost15 source files
if (IMPLEMENTATION STREQUAL SIMD)
    message(AUTHOR_WARNING "Building SIMD implementation version.")
    target_sources(libgost15 PRIVATE src/SIMD/SIMD.c src/SIMD/SIMD_tables.c src/shared/tables.c)
elseif(IMPLEMENTATION STREQUAL Optimised)
    message(AUTHOR_WARNING "Building optimised implementation version.")
    target_sources(libgost15 PRIVATE src/optimised/optimised.c src/optimised/optimised_tables.c src/shared/tables.c)
elseif(IMPLEMENTATION STREQUAL Compact)
    message(AUTHOR_WARNING "Building compact implementation version.")
    target_sources(libgost15 PRIVATE src/compact/compact.c src/shared/tables.c)
else()
    message(FATAL_ERROR "No implementation specified.")
endif()

## libgost15 include directories
target_include_directories(libgost15 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(libgost15 PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(libgost15 PRIVATE src)

## libgost15 compile feature: c_restrict
if (CMAKE_C_COMPILE_FEATURES)
    target_compile_features(libgost15 PUBLIC c_restrict)
endif()

## Falling back to strict C standard
set_target_properties(libgost15 PROPERTIES C_EXTENSIONS OFF)
set_target_properties(libgost15 PROPERTIES C_STANDARD 99)
set_target_properties(libgost15 PROPERTIES C_STANDARD_REQUIRED ON)

## Enabling coverage
if(ENABLE_COVERAGE)
    target_link_libraries(libgost15 PRIVATE --coverage)
    target_compile_options(libgost15 PRIVATE --coverage)
endif()

## libgost15 selftests
add_subdirectory(tests)
