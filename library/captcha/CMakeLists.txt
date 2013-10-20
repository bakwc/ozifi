add_library (captcha captcha.cpp)
target_link_libraries(captcha utils
                             image
                             ${Boost_LIBRARIES})
