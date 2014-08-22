#pragma once

#include "callback.h"
#include "friend.h"

/** This file contains client config declaration.
 * Client config required to construct client.
 * You need to specify all necessary parameters
 * and callbacks. */

namespace NVocal {

struct TClientConfig {
    std::string StateDir;                       // directory with internal state data
    TDataCallback CaptchaAvailableCallback;     // on captcha available (for login, reigster, etc.)
    TLoginCallback LoginResultCallback;         // on login failed / success
    TRegisterCallback RegisterResultCallback;   // on register success / fail
    TBoolCallback ConnectedCallback;            // on connection established / failed
    TNamedCallback ConferenceCallCallback;      // on incoming conference call
    TNamedCallback ConferenceJoinCallback;      // on join to conference
    TNamedCallback ConferenceLeftCallback;      // on conference left
    TMessageCallback OnMessageReceived;         // on message received
    TMessageCallback ConferenceMessageCallback; // on conference message received
    TStringCallback FriendRequestCallback;      // on friend request received (friend login)
    TCallBack FriendlistChangedCallback;        // on friendlist changed (optional)
    TFriendCallback OnFriendAdded;              // on new friend appeared in friendlist (optional)
    TFriendCallback OnFriendRemoved;            // on friend removed from friendlist (optional)
    TFriendCallback OnFriendUpdated;            // on friend info updated
    TFriendCallback OnFriendCallStatusChanged;             // on incoming call
    TOnCallResult OnCallResult;                 // on friend accepted / declined call
    TFriendDataCallback OnVideoAvailable;       // on video available
    TDataCallback OnAudioReceived;              // on audio data available
};

} // NVocal
