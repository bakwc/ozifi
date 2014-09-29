add_library(space_capture_lib defines.h
                              space.h
                              world.cpp)

target_link_libraries(space_capture_lib
                             vocal_lib)
