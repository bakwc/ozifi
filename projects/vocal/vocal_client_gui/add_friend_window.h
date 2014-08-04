#pragma once

#include <QWidget>
#include <QPointer>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class TAddFriendWindow: public QWidget {
    Q_OBJECT
public:
    explicit TAddFriendWindow();
    ~TAddFriendWindow();
signals:
    void OnAddFriend(const QString& login);
private:
    QPointer<QLabel> FriendLoginLabel;
    QPointer<QLineEdit> FriendLoginEdit;
    QPointer<QPushButton> AddButton;
};
