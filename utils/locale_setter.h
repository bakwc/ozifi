#pragma once

class __ULocaleSetter {
public:
    __ULocaleSetter();
    inline void Init() {
    }
};

extern __ULocaleSetter __LocaleSetter;
static void __LocaleSetterInitializer() {
    __LocaleSetter.Init(); // hack to force linking
}
