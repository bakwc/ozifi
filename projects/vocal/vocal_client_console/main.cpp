#include <iostream>
#include <projects/vocal/vocal_client_lib/client.h>

using namespace std;
using namespace NVocal;

void OnCaptcha(const TBuffer& data) {
    cout << "captha received: " << data.Size() << "\n";
}

int main(int argc, char** argv) {
    TClientConfig config;
    config.CaptchaAvailableCallback = &OnCaptcha;
    TClient client(config);
    return 0;
}
