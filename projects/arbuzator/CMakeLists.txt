
add_executable (arbuzator main.cpp import_remapper.cpp data_remapper.cpp)
target_link_libraries(arbuzator
                        pe_bliss
                        distorm
                        utils
                        ${Boost_LIBRARIES})
