add_library (udtlib server.cpp)
target_link_libraries(udtlib utils
                             udt4
                             ${Boost_LIBRARIES})
