add_executable(respacker respacker.cpp)
target_link_libraries(respacker
                        utils
                        ${Boost_LIBRARIES})
