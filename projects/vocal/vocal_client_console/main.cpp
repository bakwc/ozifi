#include <iostream>
#include <thread>
#include <memory>
#include <utils/string.h>
#include <projects/vocal/vocal_client_lib/client.h>

using namespace std;
using namespace std::placeholders;
using namespace NVocal;

class TVocaConsa {
public:
    TVocaConsa() {
        TClientConfig config;
        config.CaptchaAvailableCallback = std::bind(&TVocaConsa::OnCaptcha, this, _1);
        config.RegisterResultCallback = std::bind(&TVocaConsa::OnRegistered, this, _1);
        Client.reset(new TClient(config));
        Client->Register("fippo@pastexen.com");
    }
    void OnCaptcha(const TBuffer& data) {
        SaveFile("captcha.png", data.ToString());
        cout << "input captcha.png text: ";
        string captchaText;
        string password;
        cin >> captchaText;
        cout << "input password: ";
        cin >> password;
        Client->Register("fippo@pastexen.com", // login
                         password,
                         "fippo@pastexen.com", // e-mail
                         captchaText);
    }
    void OnRegistered(ERegisterResult res) {
        cout << "Register res: " << (int)res << "\n";
    }
private:
    unique_ptr<TClient> Client;
};

int main(int argc, char** argv) {
    TVocaConsa consa;
    while (true) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
    return 0;
}
