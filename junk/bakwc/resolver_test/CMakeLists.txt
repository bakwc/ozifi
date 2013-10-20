
add_executable (resolver_test main.cpp)
target_link_libraries(resolver_test
                        resolver
                        utils
                        ${Boost_LIBRARIES})
