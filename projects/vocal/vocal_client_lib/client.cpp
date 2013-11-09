#include <functional>
#include <utils/exception.h>
#include <utils/cast.h>
#include <utils/string.h>
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
    if (config.StateDir.empty()) {
        throw UException("no state dir found in config");
    }
    NUdt::TClientConfig udtConfig;
    udtConfig.ConnectionCallback = std::bind(&TClient::OnConnected, this, _1);
    udtConfig.DataReceivedCallback = std::bind(&TClient::OnDataReceived, this, _1);
    udtConfig.ConnectionLostCallback = std::bind(&TClient::OnDisconnected, this);
    Client.reset(new NUdt::TClient(udtConfig));
    try {
        LoadState();
    } catch (const UException&) {
    }
}

void TClient::LoadState() {
    string login = LoadFile(Config.StateDir + "/" + "account.txt");
    if (login.empty()) {
        throw UException("failed to load login");
    }
    StateDir = Config.StateDir + "/" + login + "/";
    string state = LoadFile(StateDir + "state");
    if (state.empty()) {
        throw UException("faile to load state");
    }
    if (!State.ParseFromString(state)) {
        throw UException("failed to deserialize state");
    }
}

void TClient::SaveState() {
    if (!State.has_login()) {
        throw UException("state not initialized");
    }
    string login = State.login();
    // todo: create dirs if they not exists
    SaveFile(Config.StateDir + "/" + "account.txt", login + "@" + State.host());
    StateDir = Config.StateDir + "/" + login + "@" + State.host() + "/";
    string state = State.SerializeAsString();
    SaveFile(StateDir + "state", state);
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
            if (registerResult == RR_Success) {
                SaveState();
            }
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
            packetStr = Decompress(packetStr);
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
                assert(!Password.empty() && "no password");
                State.set_publickey(packet.publickey());
                State.set_privatekey(DecryptSymmetrical(GenerateKey(Password), packet.encryptedprivatekey()));
                Password.clear();
                SaveState();
                Config.LoginResultCallback(packet.result());
            }
            ForceDisconnect();
        }
    } break;
    case CS_Connecting: {
        Buffer += data.ToString();
        string packetStr;
        if (Deserialize(Buffer, packetStr)) {
            packetStr = Decompress(packetStr);
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
            string response = Compress(request.SerializeAsString());
            response = Serialize(EncryptAsymmetrical(State.serverpublickey(), response));
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
    } break;
    case CS_Connected: {
        Buffer += data.ToString();
        string packetStr;
        if (Deserialize(Buffer, packetStr)) {
            packetStr = Decompress(DecryptSymmetrical(State.sessionkey(), packetStr));
            EServerPacket packetType = (EServerPacket)packetStr[0];
            packetStr = packetStr.substr(1);
            switch (packetType) {
            case SP_FriendAlreadyExists: {
                // todo: just sync friendList if required
                // send repeated authorization request
            } break;
            case SP_FriendAdded: {
                // todo: send authorization request
            } break;
               case SP_SyncMessages: {
                TClientSyncPacket packet;
                if (!packet.ParseFromString(packetStr)) {
                    throw UException("failed to parse client sync message packet");
                }
                for (size_t i = 0; i < packet.encryptedmessages_size(); ++i) {
                    OnEncryptedMessageReceived(packet.encryptedmessages(i));
                }
            } break;
            case SP_SyncInfo: {
                TClientSyncInfoPacket packet;
                if (!packet.ParseFromString(packetStr)) {
                    throw UException("failed to parse client sync info packet");
                }
                for (size_t i = 0; i < packet.friends_size(); ++i) {
                    const TSyncFriend& frnd = packet.friends(i);
                    cerr << frnd.login() << " ";
                    if (frnd.status() == AS_WaitingAuthorization) {
                        cerr << "add request\n";
                    } else if (frnd.status() == AS_UnAuthorized) {
                        cerr << "unauthorized\n";
                    }
                    cerr << "\n";
                }
            }
            }
        }
    } break;
    default:
        assert(false && "unknown state");
        break;
    }
}

void TClient::OnEncryptedMessageReceived(const string& message) {
    // todo: implement this
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

bool TClient::HasConnectData() {
    return (State.has_host() &&
            State.has_login() &&
            State.has_privatekey() &&
            State.has_publickey());
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

void TClient::Login(const std::string& password,
                    const std::string& captcha)
{
    lock_guard<mutex> guard(Lock);
    if (CurrentState != CS_Logining) {
        throw UException("not logining");
    }
    TClientLoginPacket packet;
    packet.set_login(State.login());
    packet.set_loginpasswordhash(Hash(State.login() + password));
    packet.set_captchatext(captcha);
    Password = password;
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

void TClient::Register(const std::string& preferedPassword,
                       const std::string& email,
                       const std::string& captcha)
{
    lock_guard<mutex> guard(Lock);
    if (CurrentState != CS_Registering) {
        throw UException("not logining");
    }
    TClientRegisterPacket packet;
    packet.set_login(State.login());
    packet.set_loginpasswordhash(Hash(State.login() + preferedPassword));
    packet.set_captchatext(captcha);
    packet.set_email(email);
    packet.set_publickey(State.publickey());
    packet.set_encryptedprivatekey(EncryptSymmetrical(GenerateKey(preferedPassword), State.privatekey()));
    assert(State.has_serverpublickey() && "no server public key found");
    string data =  EncryptAsymmetrical(State.serverpublickey(), Compress(packet.SerializeAsString()));
    CurrentState = CS_RegisteringConfirmWait;
    Client->Send(Serialize(data));
}

void TClient::AddFriend(const std::string& friendLogin) {
    if (CurrentState != CS_Connected) {
        throw UException("not connected");
    }
    string message(1, (ui8)RT_AddFriend);
    string friendKey = GenerateRandomSequence(16);
    TAddFriendRequest request;
    request.set_login(friendLogin);
    request.set_encryptedkey(EncryptSymmetrical(State.privatekey(), friendKey));
    message += request.SerializeAsString();
    Client->Send(Serialize(EncryptSymmetrical(State.sessionkey(), Compress(message))));
    Friends.insert(pair<string, TFriend>(friendLogin, TFriend(friendLogin, FS_Unauthorized)));
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
