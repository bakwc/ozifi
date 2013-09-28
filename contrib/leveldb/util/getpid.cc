#include <windows.h>

size_t GetPid() {
    return GetCurrentProcessId();
}

size_t GetTid() {
	return GetCurrentThreadId();
}
