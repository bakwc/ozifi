
add_definitions( -DCURL_STATICLIB )
add_library (http_fetcher fetcher.cpp async_fetcher.cpp)
target_link_libraries(http_fetcher utils
                             libcurl
                             ${Boost_LIBRARIES}
                             ${CURL_LIBRARY})
