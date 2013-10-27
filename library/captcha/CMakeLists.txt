add_library (captcha captcha.cpp)
target_link_libraries(captcha utils
                             libcaptcha
                             image
                             ${Boost_LIBRARIES})
