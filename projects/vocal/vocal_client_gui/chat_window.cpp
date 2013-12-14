#include "chat_window.h"

TChatWindow::TChatWindow(const QString& frndLogin)
    : FriendLogin(frndLogin)
{
}

void TChatWindow::ShowMessage(const QString& message) {
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
