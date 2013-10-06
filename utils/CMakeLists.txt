
add_library (utils
                string.cpp
                cast.cpp
                colonsep.cpp
                exception.cpp
                settings.cpp
                unused.h
                iostream.cpp
                locale_setter.cpp)
IF (WIN32)
target_link_libraries(utils iconv html_entities
                            ${Boost_LIBRARIES}
                            ${ICONV_LIBRARY})
ENDIF (WIN32)
IF (UNIX)
target_link_libraries(utils html_entities
                            ${Boost_LIBRARIES}
                            ${ICONV_LIBRARY})
ENDIF(UNIX)
