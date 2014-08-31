
add_executable (udt_test main.cpp)
target_link_libraries(udt_test
                        udtlib
                        utils
                        ${Boost_LIBRARIES})
