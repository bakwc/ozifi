#include "friend.h"

namespace NVocal {

const std::string& TFriend::GetLogin() {
}

const std::string& TFriend::GetName() {
}

EFriendStatus TFriend::GetStatus() {
}

const std::string& TFriend::GetStatusMessage() {
}

std::vector<TMessage> TFriend::GetHistory() {
}

void TFriend::SendMessage(const std::string& message) {
}

void TFriend::SendFile(const std::string& name,
              size_t size,
              TDataRequireCallback fileDataCallback)
{
}

void TFriend::StartCall(TDataRequireCallback videoDataRequireCallback,
               TDataRequireCallback audioDataRequireCallback,
               TDataCallback audioDataCallback,
               TDataCallback videoDataCallback,
               TBoolCallback partnerVideoStatusCallback,
               bool videoEnabled)
{
}

void TFriend::EnableVideo() {
}

void TFriend::DisableVideo() {
}

void TFriend::FinishCall() {
}

} // NVocal
