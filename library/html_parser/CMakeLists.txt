
add_library (html_parser html_parser.cpp utils.cpp)
target_link_libraries(html_parser utils
                             htmlcxx
                             ${Boost_LIBRARIES})
