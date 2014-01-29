
set(CMAKE_CXX_FLAGS "-std=c++0x")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_wrap_cpp(MOC_SOURCES world.h
                         network.h)

add_executable (space_capture_server main.cpp
                                     ${MOC_SOURCES}
                                     network.cpp
                                     world.cpp)

target_link_libraries(space_capture_server space_capture_lib)

qt5_use_modules(space_capture_server Network)
