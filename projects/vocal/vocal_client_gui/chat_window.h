#pragma once

#include <QWidget>
#include <QHash>
#include <QStringListModel>
#include <QPointer>
#include <QTextEdit>
#include <memory>

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
public:
    explicit TChatWindow(const QString& frndLogin);
    void ShowMessage(const QString& message, bool incoming);
signals:
    void SendMessage(const QString& frndLogin, const QString& message);
private slots:
    void OnSendMessage(const QString& message);
private:
    QString FriendLogin;
    QStringList Messages;
    QStringListModel MessagesModel;
    QPointer<TChatMessageEdit> MessageEdit;
};

typedef std::shared_ptr<TChatWindow> TChatWindowRef;

class TChatWindows: public QWidget {
    Q_OBJECT
public:
    void ShowChatWindow(const QString& frndLogin);
public slots:
    void ShowMessage(const QString& frndLogin, const QString& message, bool incoming);
signals:
    void SendMessage(const QString& frndLogin, const QString& message);
private:
    void CreateWindowIfMissing(const QString& frndLogin);
private:
    QHash<QString, TChatWindowRef> ChatWindows;
};
