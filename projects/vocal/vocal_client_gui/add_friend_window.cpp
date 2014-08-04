#include <QApplication>
#include <QDesktopWidget>

#include "add_friend_window.h"


TAddFriendWindow::TAddFriendWindow() {
    this->move(QApplication::desktop()->availableGeometry().center() - this->rect().center());

    this->setWindowTitle(tr("Add Friend"));

    FriendLoginLabel = new QLabel(tr("Friend login:"), this);
    FriendLoginEdit = new QLineEdit(this);
    AddButton = new QPushButton(tr("Add"),this);
    FriendLoginLabel->move(20, 25);
    FriendLoginEdit->move(20 + FriendLoginLabel->width() + 10, 20);

    this->setFixedWidth(FriendLoginEdit->x() + FriendLoginEdit->width() + 40);

    AddButton->move(this->width() / 2 - AddButton->width() / 2,
                    20 + FriendLoginEdit->height() + 10);
    this->setFixedHeight(20 + FriendLoginEdit->height() +
                         10 + AddButton->height() + 10);

    connect(AddButton.data(), &QPushButton::clicked, [this]() {
        emit OnAddFriend(FriendLoginEdit->text());
        this->hide();
    });

    this->show();
}

TAddFriendWindow::~TAddFriendWindow() {
}
