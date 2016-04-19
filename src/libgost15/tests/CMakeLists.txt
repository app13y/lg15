##########################################################################################
##                                                                                      ##
##                                 libgost15 selftests                                  ##
##                                                                                      ##
##########################################################################################

## libgost15 selftests target declaration
add_executable(selftests_gost selftests_gost.c)

## Linking libgost15
target_link_libraries(selftests_gost PUBLIC gosthopper::libgost15)

## Enabling and adding tests
enable_testing()
add_test(selftests_gost selftests_gost)