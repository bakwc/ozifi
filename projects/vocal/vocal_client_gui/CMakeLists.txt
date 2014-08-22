
set(CMAKE_CXX_FLAGS "-std=c++0x")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_wrap_cpp(MOC_SOURCES application.h
                            login_window.h
                            main_window.h
                            chat_window.h
                            add_friend_window.h
                            audio.h)

qt5_add_resources(MOC_RESOURCES statuses.qrc)

add_executable (vocal_client_gui main.cpp
                                ${MOC_SOURCES}
                                ${MOC_RESOURCES}
                                application.cpp
                                login_window.cpp
                                main_window.cpp
                                chat_window.cpp
                                add_friend_window.cpp
                                audio.cpp)

target_link_libraries(vocal_client_gui
                        udt4
                        udtlib
                        vocal_client_lib
                        ${Boost_LIBRARIES})

qt5_use_modules(vocal_client_gui Widgets Multimedia)
