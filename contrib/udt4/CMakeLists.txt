IF (WIN32)
add_definitions( -DUDT_EXPORTS "-fms-extensions -finline-functions -fno-strict-aliasing -fvisibility=hidden")
ENDIF(WIN32)

add_library(udt4
                api.cpp
                buffer.cpp
                cache.cpp
                ccc.cpp
                channel.cpp
                common.cpp
                core.cpp
                epoll.cpp
                list.cpp
                md5.cpp
                packet.cpp
                queue.cpp
                window.cpp)

IF (WIN32)
target_link_libraries(udt4 pthread wsock32 ws2_32)
ENDIF (WIN32)
IF (UNIX)
target_link_libraries(udt4 pthread)
ENDIF(UNIX)
