#include <iostream>
#include <thread>
#include <memory>
#include <utils/string.h>
#include <projects/vocal/vocal_client_lib/client.h>
#include <projects/vocal/vocal_lib/utils.h>

#define forever while(true)

using namespace std;
using namespace std::placeholders;
using namespace NVocal;

enum EStatus {
    ST_None,
    ST_Registering,
    ST_Logining,
    ST_Authorizing
};

inline char FriendStatusToChar(EFriendStatus status) {
    switch (status) {
    case FS_Offline: return '-';
    case FS_Available: return '+';
    case FS_Unauthorized: return '?';
    case FS_AddRequest: return '.';
    case FS_Away: return '+';
    case FS_Busy: return '+';
    }
}

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
        config.FriendRequestCallback = std::bind(&TVocaConsa::OnFriendRequest, this, _1);
        config.FriendlistChangedCallback = std::bind(&TVocaConsa::ShowFriends, this);
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
        MainThreadPtr.reset(new thread(std::bind(&TVocaConsa::MainThread, this)));
    }
    void MainThread() {
        while (true) {
            MainMenu();
        }
    }
    void MainMenu() {
        cout << "1 - add friend\n";
        cout << "2 - show friends\n";
        cout << "3 - start chat with friend\n";
        string choice;
        cin >> choice;
        if (choice == "1") {
            AddFriend();
        } else if (choice == "2") {
            ShowFriends();
        } else if (choice == "3") {
            StartChat();
        } else {
            cout << "wrong selectiong\n";
        }
    }
    void AddFriend() {
        string friendLogin;
        cout << "friend login: ";
        cin >> friendLogin;
        Client->AddFriend(friendLogin);
    }
    void OnFriendRequest(const string& login) {
        cout << "Friend request received: " << login << "\n";
        cout << flush;
    }
    void ShowFriends() {
        cerr << "ShowFriend()\n";
        TFriendIterator it;
        for (it = Client->FriendsBegin(); it != Client->FriendsEnd(); ++it) {
            TFriend& frnd = it->second;
            cout << FriendStatusToChar(frnd.GetStatus()) << " ";
            if (!frnd.GetName().empty()) {
                cout << frnd.GetName() << " (" << frnd.GetLogin() << ")";
            } else {
                cout << frnd.GetLogin();
            }
            cout << "\n";
        }
        cout << flush;
    }
    void StartChat() {
        string friendLogin;
        string message;
        cout << "friend login: ";
        cin >> friendLogin;
        TFriend& frnd = Client->GetFriend(friendLogin);
        forever {
            cin >> message;
            if (message.empty()) {
                break;
            }
            frnd.SendMssg(message);
        }
    }

private:
    unique_ptr<TClient> Client;
    unique_ptr<thread> MainThreadPtr;
    EStatus Status;
};

int main(int argc, char** argv) {
    TVocaConsa consa;
    forever {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
    return 0;
}
