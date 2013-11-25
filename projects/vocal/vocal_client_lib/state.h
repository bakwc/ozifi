#pragma once

namespace NVocal {

enum EClientState {
    CS_Disconnected,
    CS_Disconnecting,
    CS_Connecting,
    CS_ConnectingConfirmWait,
    CS_Logining,
    CS_LoginingConfirmWait,
    CS_Registering,
    CS_RegisteringConfirmWait,
    CS_Connected
};

} // NVocal
