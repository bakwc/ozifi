add_library (vocal_lib client.cpp message.h friend.cpp conference.cpp callback.h)
target_link_libraries(vocal_lib utils
                             udtlib
                             ${Boost_LIBRARIES})
