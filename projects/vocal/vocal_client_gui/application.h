#pragma once

#include <memory>
#include <unordered_map>
#include <QApplication>
#include <QImage>
#include <QAbstractListModel>
#include <utils/string.h>
#include <projects/vocal/vocal_client_lib/client.h>
#include <projects/vocal/vocal_lib/utils.h>
#include <QtMultimedia/QtMultimedia>

#include "main_window.h"
#include "login_window.h"
#include "chat_window.h"
#include "add_friend_window.h"

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
    void OnFriendAdded(NVocal::TFriendRef frnd);
    void OnFriendRemoved(NVocal::TFriendRef frnd);
    void OnFriendUpdated(NVocal::TFriendRef frnd);
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
public:
    QString SelfLogin();
signals:
    void BadLogin();
    void RegistrationSuccess();
    void CaptchaAvailable(QImage image);
    void RegistrationFailed(const QString& message);
    void MessageReceived(const QString& frndLogin, const QString& message, bool incoming);
private slots:
    void Register(const QString& login);
    void Login(const QString& login);
    void SendMessage(const QString& frndLogin, const QString& message);
private:
    void Authorize();
    void OnCaptcha(const TBuffer& data);
    void OnRegistered(NVocal::ERegisterResult res);
    void OnLogined(NVocal::ELoginResult res);
    void OnConnected(bool success);
    void OnMessageReceived(const NVocal::TMessage& message);
    void LaunchLogin();                 // Show login window
    void LaunchMain();                  // Show main window and connect
    void OnSuccesfullyRegistered();
    void OnFriendAdded(NVocal::TFriendRef frnd);
    void OnFriendRemoved(NVocal::TFriendRef frnd);
    void OnFriendUpdated(NVocal::TFriendRef frnd);
    void OnCallReceived(NVocal::TFriendRef frnd);
    std::string OnAudioInput(size_t size);
private:
    TImageStorage ImageStorage;
    std::unique_ptr<TFriendListModel> FriendListModel;
    std::unique_ptr<NVocal::TClient> Client;
    std::unique_ptr<TLoginWindow> LoginWindow;
    std::unique_ptr<TMainWindow> MainWindow;
    std::unique_ptr<TChatWindows> ChatWindows;
    std::unique_ptr<TAddFriendWindow> AddFriendWindow;
    QAudioFormat AudioFormat;
    QAudioDeviceInfo AudioDevice;
    std::unique_ptr<QAudioInput> AudioInput;
    EStatus Status;
};

// VocaGuiApp - pointer to single instance of TVocaGuiApp
#define VocaGuiApp (static_cast<TVocaGuiApp *>(QCoreApplication::instance()))
