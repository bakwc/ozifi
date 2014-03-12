
add_executable (arbuzator main.cpp import_remapper.cpp data_remapper.cpp instruction_remapper.cpp)
target_link_libraries(arbuzator
                        pe_bliss
                        distorm
                        utils
                        json
                        ${Boost_LIBRARIES})
