#pragma once

#include <memory>
#include <unordered_map>
#include <QApplication>
#include <QImage>
#include <utils/string.h>
#include <projects/vocal/vocal_client_lib/client.h>
#include <projects/vocal/vocal_lib/utils.h>

#include "main_window.h"
#include "login_window.h"
#include "chat_window.h"

enum EStatus {
    ST_None,
    ST_Registering,
    ST_Logining,
    ST_Authorizing
};

class TVocaGuiApp: public QApplication {
    Q_OBJECT
public:
    TVocaGuiApp(int& argc, char **argv);
    ~TVocaGuiApp();
    void AuthorizationMenu();
    void Authorize();
    void OnCaptcha(const TBuffer& data);
    void OnRegistered(NVocal::ERegisterResult res);
    void OnLogined(NVocal::ELoginResult res);
    void OnConnected(bool success);
signals:
    void BadLogin();
    void CaptchaAvailable(QImage image);
    void RegistrationFailed(const QString& message);
private slots:
    void Register(const QString& login);
    void Login(const QString& login);
    void DoLogin(const QString& captcha,
                 const QString& password);
    void DoRegister(const QString& captcha,
                    const QString& password,
                    const QString& email);
private:
    void OnSuccesfullyRegistered();
private:
    std::unique_ptr<NVocal::TClient> Client;
    std::unique_ptr<TLoginWindow> LoginWindow;
    std::unique_ptr<TMainWindow> MainWindow;
    std::unordered_map<std::string, std::unique_ptr<TChatWindow> > ChatWindows;
    EStatus Status;
};
