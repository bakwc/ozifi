
add_executable (habr_fetcher main.cpp)
target_link_libraries(habr_fetcher
                        http_fetcher
                        kwstorage
                        ${Boost_LIBRARIES})
