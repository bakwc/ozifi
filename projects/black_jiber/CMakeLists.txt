
add_executable (black_jiber main.cpp)
target_link_libraries(black_jiber
                        http_server
                        http_fetcher
                        json
                        libcurl
                        ${Boost_LIBRARIES})
