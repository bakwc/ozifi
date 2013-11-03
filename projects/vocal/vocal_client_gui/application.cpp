#include <QDebug>

#include "application.h"

using namespace std;
using namespace std::placeholders;
using namespace NVocal;

TVocaGuiApp::TVocaGuiApp(int &argc, char** argv)
    : QApplication(argc, argv)
    , Status(ST_None)
{
    TClientConfig config;
    config.CaptchaAvailableCallback = std::bind(&TVocaGuiApp::OnCaptcha, this, _1);
    config.RegisterResultCallback = std::bind(&TVocaGuiApp::OnRegistered, this, _1);
    config.LoginResultCallback = std::bind(&TVocaGuiApp::OnLogined, this, _1);
    config.ConnectedCallback = std::bind(&TVocaGuiApp::OnConnected, this, _1);
    config.StateDir = "data";
    Client.reset(new TClient(config));
    if (Client->HasConnectData()) {
        MainWindow.reset(new TMainWindow());
    } else {
        LoginWindow.reset(new TLoginWindow());
        connect(LoginWindow.get(), &TLoginWindow::Register,
                this, &TVocaGuiApp::Register);
        connect(LoginWindow.get(), &TLoginWindow::Login,
                this, &TVocaGuiApp::Login);
        connect(this, &TVocaGuiApp::BadLogin,
                LoginWindow.get(), &TLoginWindow::OnBadLogin);
        connect(this, &TVocaGuiApp::CaptchaAvailable,
                LoginWindow.get(), &TLoginWindow::OnCaptchaAvailable);
        connect(LoginWindow.get(), &TLoginWindow::DoLogin,
                this, &TVocaGuiApp::DoLogin);
        connect(LoginWindow.get(), &TLoginWindow::DoRegister,
                this, &TVocaGuiApp::DoRegister);
        connect(this, &TVocaGuiApp::RegistrationFailed,
                LoginWindow.get(), &TLoginWindow::OnRegistrationFailed);
    }
    //AuthorizationMenu();
}

TVocaGuiApp::~TVocaGuiApp() {
}

void TVocaGuiApp::AuthorizationMenu() {
    cout << "1 - registrationg\n";
    cout << "2 - login with username & password\n";
    cout << "3 - login using existing keys\n";
    string choice;
    cin >> choice;
    if (choice == "1") {
        //Registration();
    } else if (choice == "2") {
        //Login();
    } else if (choice == "3") {
        //Authorize();
    } else {
        cout << "wrong selectiong\n";
        _exit(42);
    }
}

void TVocaGuiApp::Authorize() {
    try {
        Client->Connect();
    } catch (const UException& e) {
        cout << "could not authorize: " << e.what() << "\n";
        _exit(42);
    }
}

void TVocaGuiApp::OnCaptcha(const TBuffer& data) {
    qDebug() << Q_FUNC_INFO;
    QImage captchaImage;
    captchaImage.loadFromData((const unsigned char*)data.Data(), data.Size());
    emit CaptchaAvailable(captchaImage);
}

void TVocaGuiApp::OnRegistered(ERegisterResult res) {
    if (res == RR_Success) {
        OnSuccesfullyRegistered();
    } else {
        QString msg = QString::fromStdString(RegisterResultToString(res));
        emit RegistrationFailed(msg);
    }
}

void TVocaGuiApp::OnLogined(ELoginResult res) {
    if (res == RR_Success) {
        OnSuccesfullyRegistered();
    } else {
        QString msg = QString::fromStdString(LoginResultToString(res));
        emit RegistrationFailed(msg);
    }
}

void TVocaGuiApp::OnConnected(bool success) {
    if (!success) {
        cout << "failed to authorize\n";
        _exit(42);
    }
    cout << "authorized\n";
    _exit(42);
}

void TVocaGuiApp::Register(const QString& login) {
    if (Status != ST_None) {
        return;
    }
    try {
        Status = ST_Registering;
        Client->Register(login.toStdString());
    } catch (const std::exception&) {
        Status = ST_None;
        emit BadLogin();
    }
}

void TVocaGuiApp::Login(const QString& login) {
    qDebug() << Q_FUNC_INFO;
    if (Status != ST_None) {
        return;
    }
    try {
        Status = ST_Logining;
        qDebug() << login;
        Client->Login(login.toStdString());
    } catch (const std::exception&) {
        Status = ST_None;
        emit BadLogin();
    }
}

void TVocaGuiApp::DoLogin(const QString& captcha, const QString& password) {
    Client->Login(password.toStdString(), captcha.toStdString());
    // todo: handle exceptions
}

void TVocaGuiApp::DoRegister(const QString& captcha,
                             const QString& password,
                             const QString& email)
{
    Client->Register(password.toStdString(),
                     email.toStdString(),
                     captcha.toStdString());
    // todo: handle exceptions
}

void TVocaGuiApp::OnSuccesfullyRegistered() {
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(Client->HasConnectData() && "no data to connect");
    LoginWindow.reset(nullptr);
    MainWindow.reset(new TMainWindow());
}
