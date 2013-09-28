
add_library (utils string.cpp cast.cpp colonsep.cpp exception.cpp settings.cpp)
target_link_libraries(utils iconv html_entities
                            ${Boost_LIBRARIES}
                            ${ICONV_LIBRARY})
