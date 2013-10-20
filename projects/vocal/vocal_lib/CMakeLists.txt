add_library (vocal_lib resolver.cpp)
target_link_libraries(vocal_lib utils
                             resolver
                             ${Boost_LIBRARIES})
