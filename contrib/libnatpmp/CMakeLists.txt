add_library(libnatpmp
                        getgateway.c
                        natpmp.c
                        natpmpc.c
                        wingettimeofday.c)
IF (WIN32)
target_link_libraries(libnatpmp wsock32 ws2_32 Iphlpapi)
ENDIF (WIN32)
