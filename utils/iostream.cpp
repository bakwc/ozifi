#include <streambuf>

#include "string.h"
#include "iostream.h"

namespace {

class TCoutBuf: public std::streambuf {
public:
    TCoutBuf() {
        Buff.reserve(512);
        InitLocale();
    }
    ~TCoutBuf() {
        if (!Buff.empty()) {
            Print();
        }
    }
    void Print() {
        #ifdef __WIN32__
        std::wstring wbuff = UTF8ToWide(Buff);
        std::wcout << wbuff;
        #else
        std::cout << Buff;
        #endif
        Buff.clear();
    }

    int_type overflow(int_type __c) {
        unsigned char byte = __c;
        Buff.push_back(byte);
        size_t size = Buff.size();
        // if byte > 127 => we can't convert string to UTF-8
        if (byte == 10 || (size >= 512 && byte <= 127)) {
            Print();
        }
    }
private:
    std::string Buff;
} CoutBuf;

class TCerrBuf: public std::streambuf {
public:
    TCerrBuf() {
        Buff.reserve(5);
        InitLocale();
    }

    ~TCerrBuf() {
        if (!Buff.empty()) {
            Print();
        }
    }

    void Print() {
        #ifdef __WIN32__
        std::wstring wbuff = UTF8ToWide(Buff);
        std::wcerr << wbuff;
        #else
        std::cerr << Buff;
        #endif
        Buff.clear();
    }

    int_type overflow(int_type __c) {
        unsigned char byte = __c;
        Buff.push_back(byte);
        // if byte > 127 => we can't convert string to UTF-8
        if (byte <= 127) {
            Print();
        }
    }
private:
    std::string Buff;
} CerrBuf;

}

std::ostream Cout(&::CoutBuf);
std::ostream Cerr(&::CerrBuf);
