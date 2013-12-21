#include <projects/vocal/vocal_lib/compress.h>
#include <iostream>
using namespace  NVocal;

int main() {
    TAudioCodec noobCodec;

    std::cout << noobCodec.GetFrameSize() << std::endl;

    return 0;
}
