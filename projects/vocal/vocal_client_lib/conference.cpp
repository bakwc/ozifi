#include "conference.h"

namespace NVocal {

std::string TConference::GetId() {
}

std::string TConference::GetAddress() {
}

const std::string& TConference::GetTopic() {
}

void TConference::SetTopic(const std::string& topic) {
}

void TConference::AddFriend(const std::string& friendLogin) {
}

TParticipantIterator TConference::ParticipantsBegin() {
}

TParticipantIterator TConference::ParticipantsEnd() {
}

void TConference::StartCall(TDataRequireCallback videoDataRequireCallback,
               TDataRequireCallback audioDataRequireCallback,
               TDataCallback audioDataCallback,
               TNamedDataCallback videoDataCallback,
               TNamedBoolCallback partnerVideoStatusCallback,
               bool videoEnabled)
{
}

void TConference::FinishCall() {
}

std::vector<TMessage> TConference::GetHistory() {
}

void TConference::SendMessage(const TMessage& message) {
}

} // NVocal
