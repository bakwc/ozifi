
add_executable (arbuzator main.cpp)
target_link_libraries(arbuzator
                        pe_bliss
                        distorm
                        utils
                        ${Boost_LIBRARIES})
