
add_library(hiredis async.c dict.c fmacros.h hiredis.c net.c sds.c)

IF(WIN32)
target_link_libraries(hiredis pthread ws2_32)
else()
target_link_libraries(hiredis pthread)
ENDIF()
