enable_testing()
add_executable(naive_bayes_classifier_ut tests.cpp)
target_link_libraries(naive_bayes_classifier_ut
                        gtest
                        ${Boost_LIBRARIES}
                        pthread)
add_test(naive_bayes_classifier_ut naive_bayes_classifier_ut)
