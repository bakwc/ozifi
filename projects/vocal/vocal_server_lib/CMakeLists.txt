add_library (vocal_server_lib server.cpp storage.cpp sync.cpp types.h)
target_link_libraries(vocal_server_lib utils
                             vocal_lib
                             udtlib
                             http_server
                             ${Boost_LIBRARIES})
