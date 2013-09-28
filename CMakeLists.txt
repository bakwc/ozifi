cmake_minimum_required(VERSION 2.6)
project(ozifi)

set(CMAKE_CXX_FLAGS "-std=c++0x")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.48.0 COMPONENTS locale system filesystem thread chrono REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/contrib/libcurl
                    ${Boost_INCLUDE_DIRS}
                    ${PROJECT_BINARY_DIR})

add_subdirectory(contrib)
add_subdirectory(library)
add_subdirectory(projects)
add_subdirectory(utils)
