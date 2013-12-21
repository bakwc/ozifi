
add_executable (audio_test main.cpp)
target_link_libraries(audio_test
                        utils
                        vocal_lib
                        ${Boost_LIBRARIES})
