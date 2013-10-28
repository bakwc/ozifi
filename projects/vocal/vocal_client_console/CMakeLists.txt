
add_executable (vocal_client_console main.cpp)
target_link_libraries(vocal_client_console
                        vocal_client_lib
                        ${Boost_LIBRARIES})
