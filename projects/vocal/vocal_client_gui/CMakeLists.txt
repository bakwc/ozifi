
add_definitions(${Qt5Widgets_DEFINITIONS})
set(CMAKE_CXX_FLAGS "-std=c++0x ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_wrap_cpp(MOC_SOURCES application.h
                            login_window.h
                            main_window.h
                            chat_window.h)

qt5_add_resources(MOC_RESOURCES statuses.qrc)

add_executable (vocal_client_gui main.cpp
                                ${MOC_SOURCES}
                                ${MOC_RESOURCES}
                                application.cpp
                                login_window.cpp
                                main_window.cpp
                                chat_window.cpp)

target_link_libraries(vocal_client_gui
                        udt4
                        udtlib
                        vocal_client_lib
                        ${Qt5Widgets_LIBRARIES}
                        ${Boost_LIBRARIES})
