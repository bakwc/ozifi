enable_testing()
add_executable(vocal_lib_ut tests.cpp)
target_link_libraries(vocal_lib_ut
                        vocal_lib
                        gtest
                        pthread)
add_test(vocal_lib_ut vocal_lib_ut)
