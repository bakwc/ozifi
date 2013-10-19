add_library (vocal_server_lib server.cpp storage.cpp)
target_link_libraries(vocal_server_lib utils
                             udtlib
                             http_server
                             ${Boost_LIBRARIES})
