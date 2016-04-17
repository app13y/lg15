cmake_minimum_required(VERSION 3.2)

# gosthopper project declaration
project(gosthopper VERSION 0.3.5 LANGUAGES C)

# Setting gosthopper global options
option(ENABLE_PRECALCULATIONS "Use optimised implementation with precalculated tables" OFF)
option(ENABLE_SIMD            "Enable SIMD code optimisations"                         OFF)


##########################################################################################
##                                                                                      ##
##                                    gost15 library                                    ##
##                                                                                      ##
##########################################################################################
## Setting up gost15 library sources and compiler features
add_library(gost15)
add_library(gosthopper::gost15 ALIAS gost15)

if (ENABLE_PRECALCULATIONS AND ENABLE_SIMD)
    message(WARNING "Building SIMD implementation version.")
    target_sources(gost15 PRIVATE src/SIMD.c src/tables.c src/SIMD_tables.c)
elseif(ENABLE_PRECALCULATIONS)
    message(WARNING "Building optimised implementation version.")
    target_sources(gost15 PRIVATE src/optimised.c src/tables.c src/optimised_tables.c)
else()
    message(WARNING "Building compact implementation version.")
    target_sources(gost15 PRIVATE src/compact.c src/tables.c)
endif()

target_include_directories(gost15 PUBLIC include PRIVATE src)
target_compile_features(gost15 PUBLIC c_restrict)
set_target_properties(gost15 PROPERTIES C_EXTENSIONS OFF)

## Selecting implementation version
if(ENABLE_PRECALCULATIONS)
    target_compile_definitions(gost15 PRIVATE USE_OPTIMISED_IMPLEMENTATION)
endif()

## Setting GCC specific flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(gost15 PRIVATE $<$<CONFIG:Debug>:-march=native -pedantic -Wall -Wextra -Werror -Wno-unused-function -g -O0>)
    target_compile_options(gost15 PRIVATE $<$<CONFIG:Release>:-march=native -O3>)

    if(NOT ENABLE_SIMD)
        target_compile_options(gost15 PRIVATE -mno-sse)
    endif()

## Setting Clang specific flags
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(gost15 PRIVATE $<$<CONFIG:DEBUG>:-march=native -Weverything -Werror -Wno-unused-function -Wno-cast-align -g -O0>)
    target_compile_options(gost15 PRIVATE $<$<CONFIG:RELEASE>:-march=native -O3>)

    if(NOT ENABLE_SIMD)
        message(WARNING "Disabling SIMD instructions optimisation.")
        target_compile_options(gost15 PRIVATE -mno-sse)
    endif()
endif()



##########################################################################################
##                                                                                      ##
##                                      Selftests                                       ##
##                                                                                      ##
##########################################################################################
## Selftests currently are just a target
## (CTest support in the future?)
add_executable(selftests tests/selftests_gost.c)
target_link_libraries(selftests PUBLIC gosthopper::gost15)



##########################################################################################
##                                                                                      ##
##                                      Benchmark                                       ##
##                                                                                      ##
##########################################################################################
add_executable(benchmark tests/benchmark.c)
target_link_libraries(benchmark PUBLIC gosthopper::gost15)
