#include <functional>
#include <utils/exception.h>
#include <utils/cast.h>
#include <projects/vocal/vocal_lib/resolver.h>
#include <projects/vocal/vocal_lib/utils.h>
#include <projects/vocal/vocal_lib/serializer.h>
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
    case CS_Registering: {
        if (!success) {
            Config.RegisterResultCallback(RR_ConnectionFailure);
            return;
        }
        string data;
        data.push_back((char)RT_Register);
        Client->Send(data);
    } break;
    case CS_Connecting: {
        if (!success) {
            // todo: try to reconnect several times to different servers
            Config.ConnectedCallback(success);
        }
        string data;
        data.push_back((char)RT_Authorize);
        Client->Send(data);
    }
    default:
        assert(false && "unknown state");
        break;
    }
}

void TClient::OnDataReceived(const TBuffer& data) {
    switch (CurrentState) {
    case CS_Registering: {
        Buffer += data.ToString();
        string packetStr;
        if (Deserialize(Buffer, packetStr)) {
            packetStr = Decompress(packetStr);
            TServerRegisterPacket packet;
            if (!packet.ParseFromString(packetStr)) {
                ForceDisconnect();
                Config.RegisterResultCallback(RR_ConnectionFailure);
                return;
            }
            string captcha = packet.captcha();
            State.set_serverpublickey(packet.publickey());
            Config.CaptchaAvailableCallback(captcha);
        }
    } break;
    case CS_RegisteringConfirmWait: {
        Buffer += data.ToString();
        string packetStr;
        if (Deserialize(Buffer, packetStr)) {
            packetStr = Decompress(DecryptAsymmetrical(State.privatekey(), packetStr));
            if (packetStr.size() != 1) {
                ForceDisconnect();
                Config.RegisterResultCallback(RR_ConnectionFailure);
                return;
            }
            ERegisterResult registerResult;
            registerResult = (ERegisterResult)packetStr[0];
            Config.RegisterResultCallback(registerResult);
            ForceDisconnect();
            return;
        }
    } break;
    case CS_Logining: {
        Buffer += data.ToString();
        string packetStr;
        if (Deserialize(Buffer, packetStr)) {
            packetStr = Decompress(packetStr);
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
    } break;
    case CS_Connecting: {
        Buffer += data.ToString();
        string packetStr;
        if (Deserialize(Buffer, packetStr)) {
            TServerAuthorizeRequest packet;
            if (!packet.ParseFromString(packetStr)) {
                ForceDisconnect();
                Config.ConnectedCallback(false);
                return;
            }
            if (!CheckSignature(State.serverpublickey(), packet.randomsequence(), packet.signature())) {
                ForceDisconnect();
                Config.ConnectedCallback(false);
                return;
            }
            TClientAuthorizeRequest request;
            assert(State.has_login() && "no login in state");
            request.set_login(State.login());
            request.set_randomsequencehash(Hash(packet.randomsequence()));
            string response = EncryptAsymmetrical(State.serverpublickey(), Compress(request.SerializeAsString()));
            CurrentState = CS_ConnectingConfirmWait;
            Client->Send(response);
        }
    } break;
    case CS_ConnectingConfirmWait: {
        Buffer += data.ToString();
        string packetStr;
        if (Deserialize(Buffer, packetStr)) {
            assert(State.has_privatekey());
            string sessionKey = Decompress(DecryptAsymmetrical(State.privatekey(), packetStr));
            State.set_sessionkey(sessionKey);
            CurrentState = CS_Connected;
            Config.ConnectedCallback(true);
        }
    }
    default:
        assert(false && "unknown state");
        break;
    }
}

void TClient::OnDisconnected() {
    if (CurrentState == CS_Connected) {
        CurrentState = CS_Disconnected; // todo: connection lost callback
    }
}

void TClient::ForceDisconnect() {
    CurrentState = CS_Disconnecting;
    Client->Disconnect();
}

EClientState TClient::GetState() {
}

// connection
void TClient::Connect() {
    lock_guard<mutex> guard(Lock);
    if (!State.has_host() ||
        !State.has_login() ||
        !State.has_privatekey() ||
        !State.has_publickey())
    {
        throw UException("don't have enough data");
    }
    if (CurrentState != CS_Disconnected) {
        throw UException("should be disconnected before connecting");
    }
    std::vector<TNetworkAddressRef> addresses = GetConnectionAddresses(State.login(), State.host());
    if (addresses.size() == 0) {
        throw UException("no address found for host");
    }
    // todo: random address selection
    CurrentState = CS_Connecting;
    Client->Connect(*addresses[0], false, std::bind(&TClient::OnConnected, this, _1));
}

void TClient::Disconnect() {
}

void TClient::Login(const std::string& login) { // login@service.com
    lock_guard<mutex> guard(Lock);
    State.Clear();
    if (CurrentState != CS_Disconnected) {
        throw UException("should be disconnected before logining");
    }
    pair<string, string> loginHost = GetLoginHost(login);
    State.set_login(loginHost.first);
    State.set_host(loginHost.second);
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
    assert(State.has_serverpublickey() && "no server public key found");
    string data =  EncryptAsymmetrical(State.serverpublickey(), Compress(packet.SerializeAsString()));
    CurrentState = CS_LoginingConfirmWait;
    Client->Send(Serialize(data));
}

void TClient::Register(const std::string& preferedLogin) {
    lock_guard<mutex> guard(Lock);
    State.Clear();
    if (CurrentState != CS_Disconnected) {
        throw UException("should be disconnected before logining");
    }
    pair<string, string> loginHost = GetLoginHost(preferedLogin);
    State.set_login(loginHost.first);
    State.set_host(loginHost.second);
    std::vector<TNetworkAddressRef> addresses = GetConnectionAddresses(loginHost.first, loginHost.second);
    if (addresses.size() == 0) {
        throw UException("no address found for host");
    }
    // todo: random address selection
    CurrentState = CS_Registering;
    pair<string, string> keys = GenerateKeys();
    State.set_publickey(keys.second);
    State.set_privatekey(keys.first);
    Client->Connect(*addresses[0], false, std::bind(&TClient::OnConnected, this, _1));
}

void TClient::Register(const std::string& preferedLogin,
                       const std::string& preferedPassword,
                       const std::string& email,
                       const std::string& captcha)
{
    lock_guard<mutex> guard(Lock);
    if (CurrentState != CS_Registering) {
        throw UException("not logining");
    }
    TClientRegisterPacket packet;
    packet.set_login(preferedLogin);
    packet.set_loginpasswordhash(Hash(preferedLogin + preferedPassword));
    packet.set_captchatext(captcha);
    packet.set_email(email);
    assert(State.has_serverpublickey() && "no server public key found");
    string data =  EncryptAsymmetrical(State.serverpublickey(), Compress(packet.SerializeAsString()));
    CurrentState = CS_RegisteringConfirmWait;
    Client->Send(Serialize(data));
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
