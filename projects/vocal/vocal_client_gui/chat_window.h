#pragma once

#include <QWidget>
#include <QHash>
#include <QStringListModel>
#include <QPointer>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>

#ifdef SendMessage
#undef SendMessage
#endif

const size_t CHAT_WINDOW_WIDTH = 230;
const size_t CHAT_WINDOW_HEIGHT = 320;
const size_t CHAT_MESSAGE_HEIGHT = 70;

class TChatMessageEdit: public QTextEdit {
    Q_OBJECT
public:
    TChatMessageEdit();
signals:
    void SendMessage(const QString& message);
private:
    void keyPressEvent(QKeyEvent* e);
};

class TChatWindow: public QWidget {
    Q_OBJECT
private:
    enum ECallStatus {
        CS_None,
        CS_Incoming,
        CS_Connecting,
        CS_Calling // call established
    };
public:
    explicit TChatWindow(const QString& frndLogin);
    void ShowMessage(const QString& message, bool incoming);
    void OnFriendCalled();
signals:
    void SendMessage(const QString& frndLogin, const QString& message);
    void OnStartCall(const QString& frndLogin);
    void OnFinishCall(const QString& frndLogin);
private slots:
    void OnSendMessage(const QString& message);
private:
    void OnCallClicked();
    void UpdateCallStatus();
private:
    QString FriendLogin;
    QStringList Messages;
    QStringListModel MessagesModel;
    QPointer<TChatMessageEdit> MessageEdit;
    QPointer<QPushButton> CallButton;
    QPointer<QPushButton> DeclineButton;
    QPointer<QLabel> CallStatusLabel;
    ECallStatus CallStatus = CS_None;
};

typedef std::shared_ptr<TChatWindow> TChatWindowRef;

class TChatWindows: public QWidget {
    Q_OBJECT
public:
    void ShowChatWindow(const QString& frndLogin);
public slots:
    void ShowMessage(const QString& frndLogin, const QString& message, bool incoming);
    void OnFriendCalled(const QString& frndLogin);
signals:
    void SendMessage(const QString& frndLogin, const QString& message);
    void OnStartCall(const QString& frndLogin);
    void OnFinishCall(const QString& frndLogin);
private:
    void CreateWindowIfMissing(const QString& frndLogin);
private:
    QHash<QString, TChatWindowRef> ChatWindows;
};
