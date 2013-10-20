add_library (vocal_lib resolver.cpp crypto.cpp)
target_link_libraries(vocal_lib utils
                             resolver
                             ${Boost_LIBRARIES})
