#include <functional>
#include <utils/exception.h>
#include <utils/cast.h>
#include <projects/vocal/vocal_lib/resolver.h>
#include <projects/vocal/vocal_lib/defines.h>
#include <projects/vocal/vocal_lib/utils.h>
#include <projects/vocal/vocal_lib/serializer.h>
#include <projects/vocal/vocal_lib/vocal.pb.h>
#include <projects/vocal/vocal_lib/crypto.h>
#include <projects/vocal/vocal_lib/compress.h>

#include "client.h"

using namespace std;
using namespace std::placeholders;

namespace NVocal {

TClient::TClient(const TClientConfig& config)
    : Config(config)
    , CurrentState(CS_Disconnected)
{
    NUdt::TClientConfig udtConfig;
    udtConfig.ConnectionCallback = std::bind(&TClient::OnConnected, this, _1);
    udtConfig.DataReceivedCallback = std::bind(&TClient::OnDataReceived, this, _1);
    udtConfig.ConnectionLostCallback = std::bind(&TClient::OnDisconnected, this);
    Client.reset(new NUdt::TClient(udtConfig));
    if (!config.SerializedState.empty()) {
        if (!State.ParseFromString(config.SerializedState)) {
            throw UException("failed to deserialize state");
        }
    } else {
        // todo: fill state correctly
    }
}

void TClient::OnConnected(bool success) {
    switch (CurrentState) {
    case CS_Logining: {
        if (!success) {
            Config.LoginResultCallback(LR_ConnectionFail);
            return;
        }
        string data;
        data.push_back((char)RT_Login);
        Client->Send(data);
    } break;
    default:
        assert(false && "unknown state");
        break;
    }
}

void TClient::OnDataReceived(const TBuffer& data) {
    switch (CurrentState) {
    case CS_Logining: {
        Buffer += data.ToString();
        string packetStr;
        if (Deserialize(Buffer, packetStr)) {
            TServerLoginPacket packet;
            if (!packet.ParseFromString(packetStr)) {
                ForceDisconnect();
                Config.LoginResultCallback(LR_ConnectionFail);
                return;
            }
            string captcha = packet.captcha();
            State.set_serverpublickey(packet.publickey());
            Config.CaptchaAvailableCallback(captcha);
        }
    } break;
    case CS_LoginingConfirmWait: {
        Buffer += data.ToString();
        string packetStr;
        if (Deserialize(Buffer, packetStr)) {
            TServerLoginConfirm packet;
            if (!packet.ParseFromString(packetStr)) {
                ForceDisconnect();
                Config.LoginResultCallback(LR_ConnectionFail);
                return;
            }
            if (packet.result() != LR_Success) {
                Config.LoginResultCallback(packet.result());
            } else {
                assert(packet.has_publickey() && "no public key in packet");
                assert(packet.has_encryptedprivatekey() && "no private key in packet");
                State.set_publickey(packet.publickey());
                State.set_privatekey(DecryptSymmetrical(Password, packet.encryptedprivatekey()));
                Config.LoginResultCallback(packet.result());
            }
            ForceDisconnect();
        }
    }
    default:
        assert(false && "unknown state");
        break;
    }
}

void TClient::OnDisconnected() {
    CurrentState = CS_Disconnected;
}

void TClient::ForceDisconnect() {
    CurrentState = CS_Disconnecting;
    Client->Disconnect();
}

EClientState TClient::GetState() {
}

// connection
void TClient::Connect() {
}

void TClient::Disconnect() {
}

void TClient::Login(const std::string& login) { // login@service.com
    lock_guard<mutex> guard(Lock);
    if (CurrentState != CS_Disconnected) {
        throw UException("should be disconnected before logining");
    }
    pair<string, string> loginHost = GetLoginHost(login);
    std::vector<TNetworkAddressRef> addresses = GetConnectionAddresses(loginHost.first, loginHost.second);
    if (addresses.size() == 0) {
        throw UException("no address found for host");
    }
    // todo: random address selection
    CurrentState = CS_Logining;
    Client->Connect(*addresses[0], false, std::bind(&TClient::OnConnected, this, _1));
}

void TClient::Login(const std::string& login,
                    const std::string& password,
                    const std::string& captcha)
{
    lock_guard<mutex> guard(Lock);
    if (CurrentState != CS_Logining) {
        throw UException("not logining");
    }
    TClientLoginPacket packet;
    packet.set_login(login);
    packet.set_loginpasswordhash(Hash(login + password));
    packet.set_captchatext(captcha);
    assert(State.has_serverpublickkey() && "no server public key found");
    string data =  EncryptAsymmetrical(State.serverpublickey(), Compress(packet.SerializeAsString()));
    CurrentState = CS_LoginingConfirmWait;
    Client->Send(Serialize(data));
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
