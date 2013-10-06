#ifdef __WIN32__
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

size_t GetPid() {
    #ifdef __WIN32__
    return GetCurrentProcessId();
    #else
    return getpid();
    #endif
}

size_t GetTid() {
    #ifdef __WIN32__
    return GetCurrentThreadId();
    #else
    return (long int)syscall(224);
    #endif
}
