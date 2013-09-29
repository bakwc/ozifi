#include <locale.h>
#include <iostream>
#include "locale_setter.h"

__ULocaleSetter::__ULocaleSetter() {
    #ifdef __WIN32__
    setlocale(0, "");
    #else
    setlocale(LC_CTYPE, "en_US.UTF-8");
    #endif
}

__ULocaleSetter __LocaleSetter = __ULocaleSetter();
