add_library (naive_bayes_classifier classifier.cpp)
target_link_libraries(naive_bayes_classifier utils
                                ${Boost_LIBRARIES})

add_subdirectory(ut)
