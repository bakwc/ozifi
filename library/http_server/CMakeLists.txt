
add_library (http_server server.cpp content_type.cpp)
target_link_libraries(http_server utils
                             mongoose
                             ${Boost_LIBRARIES})
