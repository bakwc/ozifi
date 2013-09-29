#include <locale.h>
#include <iostream>
#include "locale_setter.h"

void InitLocale() {
    static bool initialized = false;
    if (!initialized) {
        #ifdef __WIN32__
        setlocale(0, "");
        #else
        setlocale(LC_CTYPE, "en_US.UTF-8");
        #endif
        initialized = true;
    }
}
