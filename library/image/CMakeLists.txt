add_library (image image.cpp)
target_link_libraries(image utils
                             lodepng
                             jpeg
                             ${Boost_LIBRARIES})
