#pragma once

#include <string>
#include <assert.h>
#include <utils/types.h>

namespace NVocal {

enum ERequestType {
    RT_Register,
    RT_Login,
    RT_Authorize,
    RT_AddFriend,
    RT_RemoveFriend,
    RT_HelpConnect,
    RT_SetFriendOfflineKey,
    RT_SendMessage,
    RT_SyncMessages,
    RT_CreateConference
};

enum ERegisterResult {
    RR_ConnectionFailure,
    RR_Success,
    RR_WrongCaptcha,
    RR_WrongLogin
};

enum EAuthStatus {
  AS_UnAuthorized = 1,
  AS_WaitingAuthorization = 2,
  AS_Authorized = 3
};

enum EServerPacket {
    SP_FriendAlreadyExists,
    SP_FriendRequestSended,
    SP_FriendAdded,
    SP_FriendAddFailed,
    SP_SyncMessages,
    SP_SyncInfo,
    SP_ConnectToFriend
};

const size_t DEFAULT_RANDOM_SEQUNCE_BITS = 4096;
const size_t DEFAULT_ASSYMETRICAL_KEY_LENGTH = 4096;
const size_t CAPTCHA_WIDTH = 200; // do not modify - hardcoded
const size_t CAPTCHA_HEIGHT = 70;
const std::string CRYPTO_SALT_1 = "NVocal";
const std::string CRYPTO_SALT_2 = "distributed messenger";
const std::string CRYPTO_SALT_3 = "!!WIN";

} // NVocal
