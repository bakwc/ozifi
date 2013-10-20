add_definitions( -DCARES_STATICLIB)
add_library (resolver resolver.cpp)
target_link_libraries(resolver utils
                                c-ares
                                ${Boost_LIBRARIES})
