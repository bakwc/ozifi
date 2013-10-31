#include <iostream>
#include <thread>
#include <projects/vocal/vocal_client_lib/client.h>

using namespace std;
using namespace NVocal;

void OnCaptcha(const TBuffer& data) {
    cout << "captha received: " << data.Size() << "\n";
}

void OnRegistered(ERegisterResult res) {
    cout << "Register res: " << (int)res << "\n";
}

int main(int argc, char** argv) {
    TClientConfig config;
    config.CaptchaAvailableCallback = &OnCaptcha;
    config.RegisterResultCallback = &OnRegistered;
    TClient client(config);
    client.Register("fippo@pastexen.com");
    while (true) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
    return 0;
}
