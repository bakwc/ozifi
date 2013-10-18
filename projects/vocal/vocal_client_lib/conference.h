#pragma once

#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <string>
#include <boost/optional.hpp>

#include "message.h"
#include "callback.h"

namespace NVocal {

typedef std::unordered_set<std::string> TStringSet;
typedef TStringSet TStringSetIterator;
typedef TStringSetIterator TParticipantIterator;

class TConference {
public:
    std::string GetId();
    std::string GetAddress();
    const std::string& GetTopic();
    void SetTopic(const std::string& topic);
    void AddFriend(const std::string& friendLogin);
    TParticipantIterator ParticipantsBegin();
    TParticipantIterator ParticipantsEnd();
    void StartCall(TDataRequireCallback videoDataRequireCallback, // same function for accept call
                   TDataRequireCallback audioDataRequireCallback,
                   TDataCallback audioDataCallback,
                   TNamedDataCallback videoDataCallback,
                   TNamedBoolCallback partnerVideoStatusCallback,
                   bool videoEnabled);
    void FinishCall();
    std::vector<TMessage> GetHistory(); // old messages
    void SendMessage(const TMessage& message);
private:
    std::string Id;
    boost::optional<std::string> Name;
    std::string Topic;
    TStringSet Participants;
    std::string Server;
};

typedef std::unordered_map<std::string, TConference> TConferences;
typedef TConferences::iterator TConferenceIterator;

} // NVocal
