#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QListView>
#include <QKeyEvent>
#include <QDebug>

#include "chat_window.h"
#include "application.h"


// TChatMessageEdit

TChatMessageEdit::TChatMessageEdit() {
    setReadOnly(false);
}

void TChatWindow::OnSendMessage(const QString& message) {
    qDebug() << Q_FUNC_INFO;
    emit SendMessage(FriendLogin, message);
}

void TChatMessageEdit::keyPressEvent(QKeyEvent* e){
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        qDebug() << Q_FUNC_INFO;
        emit SendMessage(this->toPlainText());
        this->clear();
    } else {
        QTextEdit::keyPressEvent(e);
    }
}


// TChatWindow

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

void TChatWindow::ShowMessage(const QString& message, bool incoming) {
    QString messageToDisplay;
    if (incoming) { // todo: use delegate for rendering purposes
        messageToDisplay = FriendLogin + ": " + message;
    } else {
        messageToDisplay = VocaGuiApp->SelfLogin() + ": " + message;
    }
    MessagesModel.insertRow(MessagesModel.rowCount());
    QModelIndex index = MessagesModel.index(MessagesModel.rowCount() - 1);
    MessagesModel.setData(index, messageToDisplay);
}


// TChatWindows

void TChatWindows::ShowChatWindow(const QString& frndLogin) {
    CreateWindowIfMissing(frndLogin);
    ChatWindows[frndLogin]->show();
}

void TChatWindows::ShowMessage(const QString& frndLogin, const QString& message, bool incoming) {
    CreateWindowIfMissing(frndLogin);
    ChatWindows[frndLogin]->ShowMessage(message, incoming);
}

void TChatWindows::CreateWindowIfMissing(const QString& frndLogin) {
    if (ChatWindows.find(frndLogin) == ChatWindows.end()) {
        TChatWindowRef chatWindow = std::make_shared<TChatWindow>(frndLogin);
        connect(chatWindow.get(), &TChatWindow::SendMessage, this, &TChatWindows::SendMessage);
        ChatWindows.insert(frndLogin, chatWindow);
    }
}
