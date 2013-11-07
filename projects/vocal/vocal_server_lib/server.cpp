#include <utils/exception.h>
#include <utils/cast.h>
#include <library/captcha/captcha.h>
#include <projects/vocal/vocal_lib/resolver.h>
#include <projects/vocal/vocal_lib/utils.h>
#include <projects/vocal/vocal_lib/defines.h>
#include <projects/vocal/vocal_lib/serializer.h>
#include <projects/vocal/vocal_lib/compress.h>
#include <projects/vocal/vocal_lib/crypto.h>
#include <projects/vocal/vocal_lib/vocal.pb.h>

#include "server.h"

using namespace std;
using namespace std::placeholders;

namespace NVocal {

TClient::TClient(const TNetworkAddress& address)
    : Address(address)
    , Status(CS_Unknown)
    , Syncing(false)
    , NeedToResync(false)
{
}

bool TClient::AcquireSyncLock() {
    lock_guard<mutex> guard(Lock);
    if (Syncing) {
        NeedToResync = true;
        return false;
    }
    NeedToResync = false;
    Syncing = true;
    return true;
}

bool TClient::ReleaseSyncLock() {
    lock_guard<mutex> guard(Lock);
    Syncing = false;
    bool result = !NeedToResync;
    NeedToResync = false;
    return result;
}


TServer::TServer(const TServerConfig& config)
    : Config(config)
{
    ClientInfoStorage.reset(new TClientInfoStorage(config.DataDirectory + "/clients"));
    MessageStorage.reset(new TMessageStorage(config.DataDirectory + "/messages"));
    SelfStorage.reset(new TSelfStorage(config.DataDirectory + "/self"));
    if (!SelfStorage->HasKeys()) {
        SelfStorage->GenerateKeys();
    }
    NUdt::TServerConfig udtConfig;
    udtConfig.NewConnectionCallback = bind(&TServer::OnClientConnected, this, _1);
    udtConfig.DataReceivedCallback = bind(&TServer::OnDataReceived, this, _1, _2);
    udtConfig.Port = config.Port;
    Server.reset(new NUdt::TServer(udtConfig));
    NHttpServer::TSettings httpConfig(config.AdminPort);
    httpConfig.Threads = 5;
    HttpServer.reset(new NHttpServer::THttpServer(httpConfig));
}

void TServer::PrintStatus(std::ostream& out) {
}

void TServer::PrintClientStatus(const std::string& client, std::ostream& out) {
}

bool TServer::OnClientConnected(const TNetworkAddress& addr) {
    // todo: some ip filtering here
    TClientRef client = make_shared<TClient>(addr);
    assert(Clients.find(addr) == Clients.end());
    Clients[addr] = client;
    return true;
}

void TServer::OnDataReceived(const TBuffer& data, const TNetworkAddress& addr) {
    assert(Clients.find(addr) != Clients.end());
    assert(data.Size() >= 1);
    TClientRef client = Clients[addr];
    boost::optional<string> response;
    bool disconnectClient = false;
    switch (client->Status) {
    case CS_Unknown: {
        ERequestType requestType = (ERequestType)data[0];
        switch (requestType) {
        case RT_Register: {
            TCaptcha captcha = GenerateCaptcha();
            client->CaptchaText = captcha.Text;
            client->Status = CS_Registering;
            TServerRegisterPacket packet;
            packet.set_captcha(captcha.PngImage);
            packet.set_publickey(SelfStorage->GetPublicKey());
            response = Serialize(Compress(packet.SerializeAsString()));
        } break;
        case RT_Login: {
            TCaptcha captcha = GenerateCaptcha();
            client->CaptchaText = captcha.Text;
            client->Status = CS_Logining;
            TServerLoginPacket packet;
            packet.set_captcha(captcha.PngImage);
            packet.set_publickey(SelfStorage->GetPublicKey());
            response = Serialize(Compress(packet.SerializeAsString()));
        } break;
        case RT_Authorize: {
            TServerAuthorizeRequest request;
            string randomSequence = GenerateRandomSequence();
            request.set_randomsequence(randomSequence);
            request.set_signature(Sign(SelfStorage->GetPrivateKey(), randomSequence));
            response = Serialize(Compress(request.SerializeAsString()));
            client->Status = CS_Authorizing;
            client->RandomSequence = randomSequence;
        }
        }
    } break;
    case CS_Registering: {
        client->Buffer += data.ToString();
        string packetStr;
        try {
            if (Deserialize(client->Buffer, packetStr)) {
                packetStr = Decompress(DecryptAsymmetrical(SelfStorage->GetPrivateKey(), packetStr));
                TClientRegisterPacket packet;
                if (!packet.ParseFromString(packetStr)) {
                    throw UException("failed to parse string");
                }
                string resp(1, 0);
                if (packet.captchatext() != client->CaptchaText) {
                    resp[0] = (ui8)RR_WrongCaptcha;
                } else if (ClientInfoStorage->Exists(packet.login())) {
                    resp[0] = (ui8)RR_WrongLogin;
                } else {
                    TClientInfo clientInfo;
                    clientInfo.Login = packet.login();
                    clientInfo.EncryptedPrivateKey = packet.encryptedprivatekey();
                    clientInfo.LoginPasswordHash = packet.loginpasswordhash();
                    clientInfo.PublicKey = packet.publickey();
                    ClientInfoStorage->Put(clientInfo);
                    resp[0] = (ui8)RR_Success;
                }
                response = Serialize(EncryptAsymmetrical(packet.publickey(), Compress(resp)));
                disconnectClient = true;
            }
        } catch (const std::exception& e) {
            cout << "notice:\tclient registration error: " << e.what() << "\n";
            response = boost::optional<string>();
            disconnectClient = true;
        }
    } break;
    case CS_Logining: {
        client->Buffer += data.ToString();
        string packetStr;
        try {
            if (Deserialize(client->Buffer, packetStr)) {
                packetStr = Decompress(DecryptAsymmetrical(SelfStorage->GetPrivateKey(), packetStr));
                TClientLoginPacket packet;
                if (!packet.ParseFromString(packetStr)) {
                    throw UException("failed to parse string");
                }
                TServerLoginConfirm confirm;
                if (packet.captchatext() != client->CaptchaText) {
                    confirm.set_result(LR_WrongCaptcha);
                } else {
                    boost::optional<TClientInfo> clientInfo;
                    clientInfo = ClientInfoStorage->Get(packet.login());
                    if (!clientInfo.is_initialized()) {
                        confirm.set_result(LR_WrongUserOrPassword);
                    } else {
                        if (clientInfo->LoginPasswordHash != packet.loginpasswordhash()) {
                            confirm.set_result(LR_WrongUserOrPassword);
                        } else {
                            confirm.set_result(LR_Success);
                            confirm.set_encryptedprivatekey(clientInfo->EncryptedPrivateKey);
                            confirm.set_publickey(clientInfo->PublicKey);
                        }
                    }
                }
                response = Serialize(Compress(confirm.SerializeAsString()));
                disconnectClient = true;
            }
        } catch (const std::exception& e) {
            // todo: make log system
            cout << "notice:\tclient login error: " << e.what() << "\n";
            response = boost::optional<string>();
            disconnectClient = true;
        }
    } break;
    case CS_Authorizing: {
        client->Buffer += data.ToString();
        string packetStr;
        try {
            if (Deserialize(client->Buffer, packetStr)) {
                packetStr = Decompress(DecryptAsymmetrical(SelfStorage->GetPrivateKey(), packetStr));
                TClientAuthorizeRequest packet;
                if (!packet.ParseFromString(packetStr)) {
                    throw UException("failed to parse string");
                }
                boost::optional<TClientInfo> clientInfo;
                clientInfo = ClientInfoStorage->Get(packet.login());
                if (!clientInfo.is_initialized()) {
                    throw UException("no such client");
                }
                if (Hash(client->RandomSequence) != packet.randomsequencehash()) {
                    throw UException("wrong hash");
                }
                string key = GenerateKey();
                client->Login = packet.login();
                client->Status = CS_Authorized;
                client->SessionKey = key;
                client->SessionLastSync = Now();
                client->Info = *clientInfo;
                response = Serialize(EncryptAsymmetrical(clientInfo->PublicKey, Compress(key)));
            }
        } catch (const std::exception& e) {
            cout << "notice:\tclient authorization error: " << e.what() << "\n";
            response = boost::optional<string>();
            disconnectClient = true;
        }
    } break;
    case CS_Authorized: {
        client->Buffer += data.ToString();
        string packetStr;
        string responseStr;
        try {
            if (Deserialize(client->Buffer, packetStr)) {
                assert(!client->SessionKey.empty() && "Client don't have session key");
                packetStr = Decompress(DecryptSymmetrical(client->SessionKey, packetStr));
                ERequestType requestType = (ERequestType)packetStr[0];
                packetStr = packetStr.substr(1);
                switch (requestType) {
                case RT_AddFriend: {
                    TFriendInfo frnd;
                    // todo: fill (or remove) other TFriendInfo fields
                    frnd.Login = packetStr;
                    frnd.Authorized = false;
                    if (client->Info.Friends.find(frnd.Login) != client->Info.Friends.end()) {
                        responseStr.resize(1);
                        responseStr[0] = (ui8)SP_FriendAlreadyExists;
                    } else {
                        client->Info.Friends[frnd.Login] = frnd;
                        ClientInfoStorage->Put(client->Info);
                        responseStr.resize(1);
                        responseStr[0] = (ui8)SP_FriendRequestSended;
                        SendAddFriendRequest(client->Login, client->Info.PublicKey, frnd.Login);
                    }
                    responseStr += frnd.Login;
                } break;
                default:
                    throw UException("unknown request type");
                }
                response = Serialize(EncryptSymmetrical(client->SessionKey, Compress(responseStr)));
            }
        } catch (const std::exception& e) {
            cout << "notice:\tclient communication error: " << e.what() << "\n";
            response = boost::optional<string>();
            disconnectClient = true;
        }
    }
    }

    if (response.is_initialized()) {
        Server->Send(TBuffer(response.get()), addr);
    }
    if (disconnectClient) { // check if it waits for data to be send
        Server->DisconnectClient(addr);
    }
}

void TServer::OnServerDataReceived(const TBuffer& data, const string& host) {
}

void TServer::SendAddFriendRequest(const string& login,
                                   const string& pubKey,
                                   const string& frndLogin)
{
    pair<string, string> loginHost = GetLoginHost(frndLogin);
    if (loginHost.second == Config.Hostname) {
        OnAddFriendRequest(login, frndLogin);
    } else {
        string request;
        SendToServer(loginHost.second, request);
    }
}

void TServer::OnAddFriendRequest(const string& login, const string& frndLogin) {
    MessageStorage->PutFriendRequest(login, frndLogin, Now());
    SyncNewMessages(login);
}

void TServer::SendToServer(const string& host, const string& message) {
    unordered_map<std::string, TPartnerServerRef>::iterator it = Servers.find(host);
    if (it == Servers.end()) {
        TPartnerDataCallback clb = std::bind(&TServer::OnServerDataReceived, this, _1, _2);
        TPartnerServerRef serverRef = make_shared<TPartnerServer>(host, clb);
        it = Servers.insert(it, pair<string, TPartnerServerRef>(host, serverRef));
    }
    TPartnerServer& server = *it->second;
    server.Send(message);
}

void TServer::SyncMessages(const string &login, TDuration from, TDuration to) {
    auto clientIt = ClientsByLogin.find(login);
    if (clientIt == ClientsByLogin.end()) {
        return;
    }
    TClientRef client = clientIt->second;
    bool done = false;
    while (!done) { // syncing while need to sync
        if (!client->AcquireSyncLock()) {
            return; // already syncing
        }
        pair<TStringVector, TStringVector> messages = MessageStorage->GetMessages(login, from, to);
        if (messages.first.size() == 0 && messages.second.size() == 0) {
            return; // no sync required
        }
        TClientSyncPacket packet;
        for (size_t i = 0; i < messages.first.size(); ++i) {
            packet.add_messages()->set_encryptedmessage(messages.first[i]);
        }
        for (size_t i = 0; i < messages.second.size(); ++i) {
            packet.add_messages()->set_friendrequestlogin(messages.second[i]);
        }
        packet.set_from(from.MicroSeconds());
        packet.set_to(to.MicroSeconds());
        client->SessionLastSync = to;

        string data(1, (ui8)RT_SyncMessages);
        data += packet.SerializeAsString();
        data = Serialize(EncryptSymmetrical(client->SessionKey, Compress(data)));

        Server->Send(data, client->Address);

        done = client->ReleaseSyncLock();
    }

    // todo: serialize messages and send them to client
}

// sends all new messages and friend requests to given client, if it is connected
void TServer::SyncNewMessages(const string& login) {
    auto clientIt = ClientsByLogin.find(login);
    if (clientIt == ClientsByLogin.end()) {
        return;
    }
    TClientRef client = clientIt->second;
    SyncMessages(login, client->SessionLastSync, Now());
}

TPartnerServer::TPartnerServer(const string& host, TPartnerDataCallback &onDataReceived)
    : OnDataReceived(onDataReceived)
{
    // todo: do this in async mode
    std::vector<TNetworkAddressRef> addresses = GetConnectionAddresses(host);
    NUdt::TClientConfig clientConfig;
    //clientConfig.DataReceivedCallback =
    Client.reset(new NUdt::TClient(clientConfig));
    Client->Connect(*addresses[0], false,                // todo: try different addresses
            std::bind(&TPartnerServer::OnConnected, this, _1));
}

TPartnerServer::~TPartnerServer() {
}

void TPartnerServer::Send(const string& message) {
    if (Status == ST_Connected) {
        Client->Send(message);
    } else if (Messages.size() < TPartnerServer::MAX_QUEUE_SIZE) {
        Messages.push(message);
    } else {
        throw UException("can not send message");
    }
}

void TPartnerServer::OnConnected(bool success) {
    if (!success) {
        // todo: reconnect
        return;
    }
    while (!Messages.empty()) {
        Client->Send(Messages.front());
        Messages.pop();
    }
}

}
