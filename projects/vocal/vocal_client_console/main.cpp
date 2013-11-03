#include <iostream>
#include <thread>
#include <memory>
#include <utils/string.h>
#include <projects/vocal/vocal_client_lib/client.h>
#include <projects/vocal/vocal_lib/utils.h>

using namespace std;
using namespace std::placeholders;
using namespace NVocal;

enum EStatus {
    ST_None,
    ST_Registering,
    ST_Logining,
    ST_Authorizing
};

class TVocaConsa {
public:
    TVocaConsa()
        : Status(ST_None)
    {
        TClientConfig config;
        config.CaptchaAvailableCallback = std::bind(&TVocaConsa::OnCaptcha, this, _1);
        config.RegisterResultCallback = std::bind(&TVocaConsa::OnRegistered, this, _1);
        config.LoginResultCallback = std::bind(&TVocaConsa::OnLogined, this, _1);
        config.ConnectedCallback = std::bind(&TVocaConsa::OnConnected, this, _1);
        config.StateDir = "data";
        Client.reset(new TClient(config));
        AuthorizationMenu();
    }
    void AuthorizationMenu() {
        cout << "1 - registrationg\n";
        cout << "2 - login with username & password\n";
        cout << "3 - login using existing keys\n";
        string choice;
        cin >> choice;
        if (choice == "1") {
            Registration();
        } else if (choice == "2") {
            Login();
        } else if (choice == "3") {
            Authorize();
        } else {
            cout << "wrong selectiong\n";
            _exit(42);
        }
    }
    void Registration() {
        cout << "login: ";
        string login;
        cin >> login;
        Status = ST_Registering;
        Client->Register(login);
    }
    void Login() {
        cout << "login: ";
        string login;
        cin >> login;
        Status = ST_Logining;
        Client->Login(login);
    }
    void Authorize() {
        try {
            Client->Connect();
        } catch (const UException& e) {
            cout << "could not authorize: " << e.what() << "\n";
            _exit(42);
        }
    }

    void OnCaptcha(const TBuffer& data) {
        SaveFile("captcha.png", data.ToString());
        cout << "input captcha.png text: ";
        string captchaText;
        string password;
        cin >> captchaText;
        cout << "input password: ";
        cin >> password;
        if (Status == ST_Registering) {
            Client->Register(password,
                             "", // e-mail
                             captchaText);
        } else if (Status == ST_Logining) {
            Client->Login(password, captchaText);
        } else {
            cout << "wrong state\n";
            _exit(42);
        }
    }
    void OnRegistered(ERegisterResult res) {
        cout << "Register result: " << RegisterResultToString(res) << "\n";
        _exit(42);
    }
    void OnLogined(ELoginResult res) {
        cout << "Login result: " << LoginResultToString(res) << "\n";
        _exit(42);
    }
    void OnConnected(bool success) {
        if (!success) {
            cout << "failed to authorize\n";
            _exit(42);
        }
        cout << "authorized\n";
        _exit(42);
    }

private:
    unique_ptr<TClient> Client;
    EStatus Status;
};

int main(int argc, char** argv) {
    TVocaConsa consa;
    while (true) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
    return 0;
}
