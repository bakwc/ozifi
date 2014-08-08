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

void TChatMessageEdit::keyPressEvent(QKeyEvent* e){
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
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
    this->setGeometry(QDesktopWidget().availableGeometry().center().x() - (CHAT_WINDOW_WIDTH / 2),
                      QDesktopWidget().availableGeometry().center().y() - (CHAT_WINDOW_HEIGHT / 2),
                       CHAT_WINDOW_WIDTH, CHAT_WINDOW_HEIGHT);
    this->setWindowTitle(frndLogin);

    MessagesModel.setStringList(Messages);

    QVBoxLayout* currentLayout = new QVBoxLayout(this);

    CallStatusLabel = new QLabel(this);
    CallStatusLabel->setStyleSheet("QLabel { font-size: 12px; font: bold; }");
    currentLayout->addWidget(CallStatusLabel.data());
    CallStatusLabel->hide();

    CallButton = new QPushButton(this);
    currentLayout->addWidget(CallButton.data());
    connect(CallButton.data(), &QPushButton::clicked, this, &TChatWindow::OnCallClicked);

    DeclineButton = new QPushButton(this);
    currentLayout->addWidget(DeclineButton.data());
    connect(DeclineButton.data(), &QPushButton::clicked, [this] () {
        CallStatus = CS_None;
        UpdateCallStatus();
        emit OnFinishCall(FriendLogin);
    });

    UpdateCallStatus();

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

void TChatWindow::OnSendMessage(const QString& message) {
    emit SendMessage(FriendLogin, message);
}

void TChatWindow::OnCallClicked() {
    if (CallStatus == CS_None) {
        CallStatus = CS_Connecting;
        UpdateCallStatus();
        emit OnStartCall(FriendLogin);
    } else if (CallStatus == CS_Calling || CallStatus == CS_Connecting) {
        CallStatus = CS_None;
        UpdateCallStatus();
        emit OnFinishCall(FriendLogin);
    } else if (CallStatus == CS_Incoming) {
        CallStatus = CS_Calling;
        UpdateCallStatus();
        emit OnStartCall(FriendLogin);
    } else {
        throw UException("wrong status");
    }
}

void TChatWindow::UpdateCallStatus() {
    CallStatusLabel->hide();
    DeclineButton->hide();
    switch (CallStatus) {
    case CS_None:
        CallButton->setText(tr("Call"));
        break;
    case CS_Connecting:
        CallStatusLabel->show();
        CallStatusLabel->setText(tr("Calling..."));
        CallButton->setText(tr("Drop"));
        break;
    case CS_Incoming:
        CallStatusLabel->show();
        CallStatusLabel->setText(tr("Incoming call..."));
        CallButton->setText(tr("Accept"));
        DeclineButton->setText(tr("Decline"));
        DeclineButton->show();
        break;
    case CS_Calling:
        CallStatusLabel->show();
        CallStatusLabel->setText(tr("Call in progress"));
        CallButton->setText(tr("Drop"));
    }
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
