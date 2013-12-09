#pragma once

#include <memory>
#include <unordered_map>
#include <QApplication>
#include <QImage>
#include <QAbstractListModel>
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

enum EFriendStatus {
    FS_Unauthorized,
    FS_Offline,
    FS_Busy,
    FS_Away,
    FS_Available,

    FS_Count
};

Q_STATIC_ASSERT(FS_Count == 5);

struct TFriendData {
    QString Login;
    EFriendStatus Status;
};

Q_DECLARE_METATYPE(TFriendData)

class TFriendListModel: public QAbstractListModel {
    Q_OBJECT
public:
    TFriendListModel(NVocal::TClient& vocalClient);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    void OnFriendAdded(const NVocal::TFriendRef& frnd);
    void OnFriendRemoved(const NVocal::TFriendRef& frnd);
    void OnFriendUpdated(const NVocal::TFriendRef& frnd);
private:
    QVector<NVocal::TFriendRef> Friends;
    NVocal::TClient& VocalClient;
};

class TImageStorage { // todo: make singleton
public:
    TImageStorage();
    const QImage& GetStatusImage(EFriendStatus status);
private:
    QVector<QImage> StatusImages;
};

class TVocaGuiApp: public QApplication {
    Q_OBJECT
public:
    TVocaGuiApp(int& argc, char **argv);
    ~TVocaGuiApp();
    void Authorize();
    void OnCaptcha(const TBuffer& data);
    void OnRegistered(NVocal::ERegisterResult res);
    void OnLogined(NVocal::ELoginResult res);
    void OnConnected(bool success);
signals:
    void BadLogin();
    void RegistrationSuccess();
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
    void LaunchLogin();                 // Show login window
    void LaunchMain();                  // Show main window and connect
    void OnSuccesfullyRegistered();
    void OnFriendAdded(const NVocal::TFriendRef& frnd);
    void OnFriendRemoved(const NVocal::TFriendRef& frnd);
    void OnFriendUpdated(const NVocal::TFriendRef& frnd);
private:
    TImageStorage ImageStorage;
    std::unique_ptr<TFriendListModel> FriendListModel;
    std::unique_ptr<NVocal::TClient> Client;
    std::unique_ptr<TLoginWindow> LoginWindow;
    std::unique_ptr<TMainWindow> MainWindow;
    std::unordered_map<std::string, std::unique_ptr<TChatWindow> > ChatWindows;
    EStatus Status;
};
