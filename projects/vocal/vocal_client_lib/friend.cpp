#include "friend.h"

namespace NVocal {

TFriend::TFriend(const std::string& login, EFriendStatus status)
    : Login(login)
    , Status(status)
{
}

const std::string& TFriend::GetLogin() {
    return Login;
}

const std::string& TFriend::GetName() {
    return Name;
}

EFriendStatus TFriend::GetStatus() {
    return Status;
}

const std::string& TFriend::GetStatusMessage() {
    assert(false && "unimplemented");
}

std::vector<TMessage> TFriend::GetHistory() {
    assert(false && "unimplemented");
}

void TFriend::SendMssg(const std::string& message) {
    assert(false && "unimplemented");
}

void TFriend::SendFile(const std::string& name,
              size_t size,
              TDataRequireCallback fileDataCallback)
{
    assert(false && "unimplemented");
}

void TFriend::StartCall(TDataRequireCallback videoDataRequireCallback,
               TDataRequireCallback audioDataRequireCallback,
               TDataCallback audioDataCallback,
               TDataCallback videoDataCallback,
               TBoolCallback partnerVideoStatusCallback,
               bool videoEnabled)
{
    assert(false && "unimplemented");
}

void TFriend::EnableVideo() {
    assert(false && "unimplemented");
}

void TFriend::DisableVideo() {
    assert(false && "unimplemented");
}

void TFriend::FinishCall() {
    assert(false && "unimplemented");
}

} // NVocal
