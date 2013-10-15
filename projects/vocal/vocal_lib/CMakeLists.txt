add_library (vocal_lib client.cpp)
target_link_libraries(vocal_lib utils
                             udtlib
                             ${Boost_LIBRARIES})
