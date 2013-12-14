#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QListView>
#include <QKeyEvent>
#include <QDebug>

#include "chat_window.h"

void TChatMessageEdit::keyPressEvent(QKeyEvent* e){
    if (e->key() == Qt::Key_Enter) {
        emit SendMessage(this->toPlainText());
        this->clear();
    }
}

TChatWindow::TChatWindow(const QString& frndLogin)
    : FriendLogin(frndLogin)
{
    qDebug() << Q_FUNC_INFO;
    this->setGeometry(QDesktopWidget().availableGeometry().center().x() - (CHAT_WINDOW_WIDTH / 2),
                      QDesktopWidget().availableGeometry().center().y() - (CHAT_WINDOW_HEIGHT / 2),
                       CHAT_WINDOW_WIDTH, CHAT_WINDOW_HEIGHT);
    this->setWindowTitle(frndLogin);

    MessagesModel.setStringList(Messages);

    QVBoxLayout* currentLayout = new QVBoxLayout(this);
    QListView* messagesListView = new QListView();
    messagesListView->setModel(&MessagesModel);
    currentLayout->addWidget(messagesListView);
    MessageEdit = new TChatMessageEdit();
    connect(MessageEdit.data(), &TChatMessageEdit::SendMessage, this, &TChatWindow::OnSendMessage);

    MessageEdit->setFixedHeight(CHAT_MESSAGE_HEIGHT);
    currentLayout->addWidget(MessageEdit);
    this->show();
}

void TChatWindow::ShowMessage(const QString& message) {
    qDebug() << Q_FUNC_INFO;
    Messages.append(FriendLogin + ": " + message);
}

void TChatWindows::ShowChatWindow(const QString& frndLogin) {
    CreateWindowIfMissing(frndLogin);
    ChatWindows[frndLogin]->show();
}

void TChatWindows::ShowMessage(const QString& frndLogin, const QString& message) {
    CreateWindowIfMissing(frndLogin);
    ChatWindows[frndLogin]->ShowMessage(message);
}

void TChatWindows::CreateWindowIfMissing(const QString& frndLogin) {
    if (ChatWindows.find(frndLogin) == ChatWindows.end()) {
        ChatWindows.insert(frndLogin, std::make_shared<TChatWindow>(frndLogin));
    }
}


void TChatWindow::OnSendMessage(const QString& message) {
    emit SendMessage(FriendLogin, message);
}
