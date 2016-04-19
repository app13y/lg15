##########################################################################################
##                                                                                      ##
##                                 libgost15 benchmark                                  ##
##                                                                                      ##
##########################################################################################

add_executable(benchmark src/benchmark.c)
target_link_libraries(benchmark gosthopper::libgost15)