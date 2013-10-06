
add_library (mongoose mongoose.c)

IF (WIN32)
target_link_libraries(mongoose pthread wsock32)
ENDIF (WIN32)

IF (UNIX)
target_link_libraries(mongoose pthread dl)
ENDIF (UNIX)

