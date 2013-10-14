add_library (vocal_lib client.cpp server.cpp)
target_link_libraries(vocal_lib utils
                             udt4
                             ${Boost_LIBRARIES})
