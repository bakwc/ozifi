add_executable(gen_captcha main.cpp)
target_link_libraries(gen_captcha
                        utils
                        captcha)
