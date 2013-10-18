#include "client.h"

namespace NVocal {

TClient::TClient(const TClientConfig& config) {
}

EClientState TClient::GetState() {
}

// connection
void TClient::Connect() {
}

void TClient::Disconnect() {
}

void TClient::Login(const std::string& login) {
}

void TClient::Login(const std::string& login,
                    const std::string& password,
                    const std::string& captcha)
{
}

void TClient::Register(const std::string& preferedLogin) {
}

void TClient::Register(const std::string& preferedLogin,
                       const std::string& preferedPassword,
                       const std::string& email,
                       const std::string& captcha)
{
}

void TClient::AddFriend(const std::string& friendLogin) {
}

void TClient::AddFriend(const std::string& friendLogin,
                        const std::string& requestMessage,
                        const std::string& captcha)
{
}

void TClient::RemoveFriend(const std::string& friendLogin) {
}

TFriend& TClient::GetFriend(const std::string& login) {
}

TFriendIterator TClient::FriendsBegin() {
}

TFriendIterator TClient::FriendsEnd() {
}

// conference
void TClient::CreateConference() {
}

void TClient::LeaveConference(const std::string& id) {
}

TConference& TClient::GetConference(const std::string& id) {
}

TConferenceIterator TClient::ConferencesBegin() {
}

TConferenceIterator TClient::ConferencesEnd() {
}

}
