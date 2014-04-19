
add_executable (habr_extractor main.cpp)
target_link_libraries(habr_extractor
                        html_parser
                        json
                        ${Boost_LIBRARIES})
