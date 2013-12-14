#pragma once

#include <QWidget>
#include <QHash>
#include <memory>

class TChatWindow: public QWidget {
    Q_OBJECT
public:
    explicit TChatWindow(const QString& frndLogin);
    void ShowMessage(const QString& message);
private:
    QString FriendLogin;
};

typedef std::shared_ptr<TChatWindow> TChatWindowRef;

class TChatWindows: public QWidget {
    Q_OBJECT
public:
    void ShowChatWindow(const QString& frndLogin);
public slots:
    void ShowMessage(const QString& frndLogin, const QString& message);
signals:
    void SendMessage(const QString& frndLogin, const QString& message);
private:
    void CreateWindowIfMissing(const QString& frndLogin);
private:
    QHash<QString, TChatWindowRef> ChatWindows;
};
