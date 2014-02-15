
set(CMAKE_CXX_FLAGS "-std=c++0x")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_wrap_cpp(MOC_SOURCES display.h network.h control.h world.h graphic_manager.h main_menu.h application.h world_display.h)
qt5_add_resources(RESOURCE_SOURCES resources.qrc)

add_executable(space_capture_client main.cpp
                                    ${RESOURCE_SOURCES}
                                    ${MOC_SOURCES}
                                    display.cpp
                                    network.cpp
                                    control.cpp
                                    world.cpp
                                    graphic_manager.cpp
                                    main_menu.cpp
                                    application.cpp
                                    world_display.cpp
                                    sphere.h)
IF (WIN32)
target_link_libraries(space_capture_client
                                    space_capture_lib
                                    http_fetcher
                                    opengl32)
ENDIF (WIN32)
IF (UNIX)
target_link_libraries(space_capture_client
                                    space_capture_lib
                                    http_fetcher
                                    GL)
ENDIF(UNIX)

qt5_use_modules(space_capture_client Widgets Network OpenGL)
