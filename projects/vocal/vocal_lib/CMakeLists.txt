add_library (vocal_lib vocal.cpp)
target_link_libraries(vocal_lib utils
                             ${Boost_LIBRARIES})
