cmake_minimum_required(VERSION 2.6)
project(ozifi)

set(CMAKE_CXX_FLAGS "-std=c++0x")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.48.0 COMPONENTS locale system filesystem thread chrono REQUIRED)

find_package(Qt5Core)

include_directories(${PROJECT_SOURCE_DIR}/contrib/leveldb/include)
include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/contrib/
                    ${PROJECT_SOURCE_DIR}/contrib/libcurl
                    ${Boost_INCLUDE_DIRS}
                    ${PROJECT_BINARY_DIR}
                    ${Qt5Widgets_INCLUDE_DIRS})
include_directories(/home/mikhail/Downloads/Dev/Boost/boost_1_55_0/)

link_directories(${PROJECT_BINARY_DIR})

add_subdirectory(contrib)
add_subdirectory(library)
add_subdirectory(projects)
add_subdirectory(junk)
add_subdirectory(utils)
