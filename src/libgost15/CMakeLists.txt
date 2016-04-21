##########################################################################################
##                                                                                      ##
##                                  libgost15 library                                   ##
##                                                                                      ##
##########################################################################################

cmake_minimum_required(VERSION 3.2)

## libgost15 project declaration
project(libgost15 VERSION 0.3.5 LANGUAGES C)

## libgost15 global options
option(ENABLE_PRECALCULATIONS "Use optimised implementation with precalculated tables" OFF)
option(ENABLE_SIMD            "Enable SIMD code optimisations"                         OFF)

## libgost15 definition
add_library(libgost15)

## libgost15 source files
if (ENABLE_PRECALCULATIONS AND ENABLE_SIMD)
    message(WARNING "Building SIMD implementation version.")
    target_sources(libgost15 PRIVATE src/SIMD.c src/tables.c src/SIMD_tables.c)
elseif(ENABLE_PRECALCULATIONS)
    message(WARNING "Building optimised implementation version.")
    target_sources(libgost15 PRIVATE src/optimised.c src/tables.c src/optimised_tables.c)
else()
    message(WARNING "Building compact implementation version.")
    target_sources(libgost15 PRIVATE src/compact.c src/tables.c)
endif()

## libgost15 include directories and compiler features
target_include_directories(libgost15 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(libgost15 PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(libgost15 PRIVATE src)

## libgost15 compile feature: c_restrict
if (CMAKE_C_COMPILE_FEATURES)
    target_compile_features(libgost15 PUBLIC c_restrict)
endif()

## Falling back to strict C standard
set_target_properties(libgost15 PROPERTIES C_EXTENSIONS OFF)

## libgost15 GCC compiler-specific flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(libgost15 PRIVATE $<$<CONFIG:Debug>:-pedantic -Wall -Wextra -Werror -Wno-unused-function -Wno-cast-align>)
    target_compile_options(libgost15 PRIVATE $<$<CONFIG:Release>:-march=native>)

    if(NOT ENABLE_SIMD)
        target_compile_options(libgost15 PRIVATE -mno-sse)
    endif()

## libgost15 Clang compiler-specific flags
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(libgost15 PRIVATE $<$<CONFIG:DEBUG>:-Weverything -Werror -Wno-unused-function -Wno-cast-align>)
    target_compile_options(libgost15 PRIVATE $<$<CONFIG:RELEASE>:-march=native>)

    if(NOT ENABLE_SIMD)
        target_compile_options(libgost15 PRIVATE -mno-sse)
    endif()
endif()

## libgost15 selftests
add_subdirectory(tests)
