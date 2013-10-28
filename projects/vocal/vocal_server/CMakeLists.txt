
add_executable (vocal_server main.cpp)
target_link_libraries(vocal_server
                        vocal_server_lib
                        utils
                        ${Boost_LIBRARIES})
