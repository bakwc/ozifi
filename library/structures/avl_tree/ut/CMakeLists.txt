enable_testing()
add_executable(avl_tree_ut tests.cpp)
target_link_libraries(avl_tree_ut
                        gtest
                        pthread)
add_test(avl_tree_ut avl_tree_ut)
