#include <utils/exception.h>
#include <utils/cast.h>
#include <library/captcha/captcha.h>
#include <projects/vocal/vocal_lib/resolver.h>
#include <projects/vocal/vocal_lib/utils.h>
#include <projects/vocal/vocal_lib/defines.h>
#include <projects/vocal/vocal_lib/serializer.h>
#include <projects/vocal/vocal_lib/compress.h>
#include <projects/vocal/vocal_lib/crypto.h>

#include "server.h"

using namespace std;
using namespace std::placeholders;

namespace NVocal {

TClient::TClient(const TNetworkAddress& address)
    : Address(address)
    , Status(CS_Unknown)
{
}

TClient::~TClient() {
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
    udtConfig.ConnectionLostCallback = bind(&TServer::OnClientDisconnected, this, _1);
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
    OnClientDisconnected(addr);
    assert(Clients.find(addr) == Clients.end());
    Clients[addr] = client;
    return true;
}

void TServer::OnClientDisconnected(const TNetworkAddress& addr) {
    auto cliIt = Clients.find(addr);
    TClientRef cli;
    if (cliIt != Clients.end()) {
        cli = cliIt->second;
        Clients.erase(cliIt);
    }
    if (!cli || cli->Login.empty()) {
        return;
    }
    auto cliByLoginIt = ClientsByLogin.find(cli->Login);
    if (cliByLoginIt == ClientsByLogin.end()) {
        return;
    }
    TClientRef cliByLogin = cliByLoginIt->second;
    if (cliByLogin->Address == cli->Address) {
        ClientsByLogin.erase(cliByLoginIt);
    }
}

void TServer::OnDataReceived(const TBuffer& data, const TNetworkAddress& addr) {
    assert(Clients.find(addr) != Clients.end());
    assert(data.Size() >= 1);
    vector<TCallBack> next;
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
        } break;
        case RT_HelpConnect: {
            TServerAuthorizeRequest request;
            string randomSequence = GenerateRandomSequence();
            request.set_randomsequence(randomSequence);
            request.set_signature(Sign(SelfStorage->GetPrivateKey(), randomSequence));
            response = Serialize(Compress(request.SerializeAsString()));
            client->Status = CS_AuthorizingHelpConnect;
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
                    clientInfo = ClientInfoStorage->Get(packet.login()); // todo: validate login
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
                clientInfo = ClientInfoStorage->Get(packet.login()); // todo: validate login
                if (!clientInfo.is_initialized()) {
                    throw UException("no such client");
                }
                if (Hash(client->RandomSequence) != packet.randomsequencehash()) {
                    throw UException("wrong hash");
                }
                if (!CheckSignature(clientInfo->PublicKey, packet.randomsequencehash(), packet.randomsequencehashsignature())) {
                    throw UException("wrong signature");
                }
                string key = GenerateKey();
                client->Login = packet.login();
                client->Status = CS_Authorized;
                client->SessionKey = key;
                client->SessionLastSync = Now();
                client->Info = *clientInfo;
                response = Serialize(EncryptAsymmetrical(clientInfo->PublicKey, Compress(key)));
                ClientsByLogin[client->Login] = client;
                next.push_back(std::bind(&TServer::SyncClientInfo, this, client->Info));
            }
        } catch (const std::exception& e) {
            cout << "notice:\tclient authorization error: " << e.what() << "\n";
            response = boost::optional<string>();
            disconnectClient = true;
        }
    } break;
    case CS_AuthorizingHelpConnect: {
        client->Buffer += data.ToString();
        string packetStr;
        try {
            if (Deserialize(client->Buffer, packetStr)) {
                packetStr = Decompress(DecryptAsymmetrical(SelfStorage->GetPrivateKey(), packetStr));
                TClientConnectHelpAuthorizeRequest packet;
                if (!packet.ParseFromString(packetStr)) {
                    throw UException("failed to parse string");
                }

                bool acceptingConnection = packet.acceptingconnection();
                if (acceptingConnection) {

                    string login = GetLoginHost(packet.login()).first;
                    auto cliIt = ClientsByLogin.find(login);
                    if (cliIt != ClientsByLogin.end()) {
                        TClientRef& cli = cliIt->second;
                        auto frndConnIt = cli->FriendConnections.find(packet.friendlogin());
                        if (frndConnIt != cli->FriendConnections.end()) {
                            TClientRef& frndConn = frndConnIt->second;
                            /** frndConn -> c2 connection;
                             *  client   -> c12 connection  (current)
                             *  cli      -> c1 connection */

                            TServerConnectHelpRequest clientHelpRequest;
                            TServerConnectHelpRequest friendHelpRequest;

                            string clientAddress = client->Address.ToString();
                            string frndAddress = frndConn->Address.ToString();
                            if (packet.has_publicaddress()) {
                                clientAddress = packet.publicaddress();
                                clientHelpRequest.set_connectiontype(CTP_Listen);
                                friendHelpRequest.set_connectiontype(CTP_Connect);
                                friendHelpRequest.set_address(clientAddress);
                            } else if (frndConn->PublicAddress.is_initialized()) {
                                frndAddress = frndConn->PublicAddress->ToString();
                                clientHelpRequest.set_connectiontype(CTP_Connect);
                                clientHelpRequest.set_address(frndAddress);
                                friendHelpRequest.set_connectiontype(CTP_Listen);
                            } else {
                                clientHelpRequest.set_connectiontype(CTP_NatTraversal);
                                clientHelpRequest.set_address(frndAddress);
                                friendHelpRequest.set_connectiontype(CTP_NatTraversal);
                                friendHelpRequest.set_address(clientAddress);
                            }
                            response = Serialize(EncryptAsymmetrical(cli->Info.PublicKey,
                                                 Compress(clientHelpRequest.SerializeAsString())));
                            Server->Send(Serialize(EncryptAsymmetrical(frndConn->Info.PublicKey,
                                         Compress(friendHelpRequest.SerializeAsString()))),
                                         frndConn->Address);
                        }
                    }
                    disconnectClient = true;

                } else {
                    string friendLogin = GetLoginHost(packet.friendlogin()).first;
                    boost::optional<TClientInfo> clientInfo;
                    clientInfo = ClientInfoStorage->Get(friendLogin);
                    if (!clientInfo.is_initialized()) {
                        throw UException("no such client");
                    }

                    if (Hash(client->RandomSequence) != packet.randomsequencehash()) {
                        throw UException("wrong hash");
                    }

                    auto friendIt = clientInfo->Friends.find(packet.login());
                    if (friendIt == clientInfo->Friends.end()) {
                        throw UException("no such friend in friend-list");
                    }

                    TFriendInfo& frnd = friendIt->second;
                    if (frnd.AuthStatus != AS_Authorized) {
                        throw UException("client not authorized");
                    }

                    assert(frnd.PublicKey.size() != 0 && "friend has no public key");
                    if (!CheckSignature(frnd.PublicKey, packet.randomsequencehash(), packet.randomsequencehashsignature())) {
                        throw UException("wrong signature");
                    }

                    auto cliIt = ClientsByLogin.find(friendLogin);
                    if (cliIt != ClientsByLogin.end()) {
                        // if friend is online - send him connect request
                        TClientRef& cli = cliIt->second;
                        /** client -> c2 connection (current)
                         *  cli    -> c1 connection
                         *  frnd   -> c2 info */
                        string connectRequestData;
                        connectRequestData.resize(1);
                        connectRequestData[0] = (ui8)SP_ConnectToFriend;
                        connectRequestData += packet.login();
                        connectRequestData = Serialize(EncryptSymmetrical(cli->SessionKey, Compress(connectRequestData)));
                        Server->Send(connectRequestData, cli->Address);
                        client->Info.PublicKey = frnd.PublicKey;
                        if (packet.has_publicaddress()) {
                            client->PublicAddress = TNetworkAddress(packet.publicaddress());
                        }
                        cli->FriendConnections[packet.login()] = client;
                    } else {
                        TServerConnectHelpRequest connectHelpRequest;
                        connectHelpRequest.set_connectiontype(CTP_Offline);
                        response = Serialize(EncryptAsymmetrical(frnd.PublicKey, Compress(connectHelpRequest.SerializeAsString())));
                        disconnectClient = true;
                    }
                }
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
                    TAddFriendRequest request;
                    if (!request.ParseFromString(packetStr)) {
                        throw UException("failed to parse addfriendrequest");
                    }
                    auto frndIt = client->Info.Friends.find(request.login());
                    if (frndIt == client->Info.Friends.end()) {
                        TFriendInfo& frnd = client->Info.Friends[request.login()];
                        frnd.Login = request.login();
                        frnd.Type = FT_Friend;
                        frnd.AuthStatus = AS_UnAuthorized;
                        frnd.EncryptedKey = request.encryptedkey();
                    } else {
                        TFriendInfo& frnd = frndIt->second;
                        if (frnd.AuthStatus == AS_WaitingAuthorization) {
                            frnd.AuthStatus = AS_Authorized;
                            frnd.EncryptedKey = request.encryptedkey();
                            frnd.NeedOfflineKey = true;
                        }
                    }
                    responseStr.resize(1);
                    try {
                        SendAddFriendRequest(client->Login, client->Info.PublicKey, request.login());
                        responseStr[0] = (ui8)SP_FriendRequestSended;
                    } catch (const UException&) {
                        responseStr[0] = (ui8)SP_FriendAddFailed;
                        client->Info.Friends.erase(request.login());
                    }
                    SyncClientInfo(client->Info);

                    responseStr += packetStr;
                } break;
                case RT_SetFriendOfflineKey: {
                    TFriendOfflineKey friendOfflineKey;
                    if (!friendOfflineKey.ParseFromString(packetStr)) {
                        throw UException("failed to parse setFriendOfflineKey");
                    }
                    if (client->Info.Friends.find(friendOfflineKey.login()) ==
                        client->Info.Friends.end())
                    {
                        throw UException("friend not found");
                    }
                    SendSetFriendOfflineKeyRequest(client->Login, friendOfflineKey);
                } break;
                case RT_SendMessage: {
                    TOfflineMessage offlineMessage;
                    if (!offlineMessage.ParseFromString(packetStr)) {
                        throw UException("failed to parse offline message");
                    }
                    auto frndIt = client->Info.Friends.find(offlineMessage.friendlogin());
                    if (frndIt == client->Info.Friends.end()) {
                        throw UException("friend not found");
                    }
                    TFriendInfo frnd = frndIt->second;
                    if (frnd.AuthStatus != AS_Authorized) {
                        throw UException("friend not authorized");
                    }
                    SendOfflineMessage(client->Login, offlineMessage.friendlogin(), offlineMessage.encryptedmessage());
                } break;
                case RT_SyncMessages: {
                    TSyncMessagesRequest syncRequest;
                    if (!syncRequest.ParseFromString(packetStr)) {
                        throw UException("failed to parse syn request");
                    }
                    TDuration from(syncRequest.from());
                    TDuration to(syncRequest.to());
                    SyncMessages(client->Login, from, to);
                } break;
                default:
                    throw UException("unknown request type");
                }
                if (!responseStr.empty()) {
                    response = Serialize(EncryptSymmetrical(client->SessionKey, Compress(responseStr)));
                }
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

    for (size_t i = 0; i < next.size(); ++i) {
        next[i]();
    }
}

void TServer::OnServerDataReceived(const TBuffer& data, const string& host) {
}

void TServer::SendAddFriendRequest(const string& login,
                                   const string& pubKey,
                                   const string& frndLogin)
{
    if (login == frndLogin) {
        throw UException("wrong friend login");
    }
    pair<string, string> loginHost = GetLoginHost(frndLogin);
    if (loginHost.second == Config.Hostname) {
        OnAddFriendRequest(loginHost.first, login + "@" + Config.Hostname,
                           pubKey, SelfStorage->GetPublicKey());
    } else {
        string request;
        // todo: fill request
        SendToServer(loginHost.second, request);
    }
}

void TServer::SendSetFriendOfflineKeyRequest(const string& login,
                                             const TFriendOfflineKey& offlineKeyPacket)
{
    pair<string, string> loginHost = GetLoginHost(offlineKeyPacket.login()); // user@host.com
    if (loginHost.second == Config.Hostname) {
        OnFriendOfflineKeyRequest(loginHost.first,
                                  login + "@" + Config.Hostname,
                                  offlineKeyPacket.offlinekey(),
                                  offlineKeyPacket.offlinekeysignature());
    }
}

void TServer::SendOfflineMessage(const string& login,
                                 const string& friendLogin,
                                 const string& message)
{
    MessageStorage->Put(login, "o" + Serialize(friendLogin) + Serialize(message), Now());  // store outgoing message
    SyncNewMessages(login);
    pair<string, string> loginHost = GetLoginHost(friendLogin);
    if (loginHost.second == Config.Hostname) {
        OnOfflineMessageReceived(loginHost.first, login + "@" + Config.Hostname, message);
    } else {
        string request;
        // todo: fill request
        SendToServer(loginHost.second, request);
    }
}

void TServer::OnAddFriendRequest(const string& login, const string& frndLogin,
                                 const string& pubKey, const string& serverPubKey)
{
    // todo: thread-safe storage access
    boost::optional<TClientInfo> info = ClientInfoStorage->Get(login);
    if (!info.is_initialized()) {
        throw UException("client not exists");
    }
    TFriendList& friends = info->Friends;
    auto frndIt = friends.find(frndLogin);
    if (frndIt == friends.end()) {
        TFriendInfo& frnd = friends[frndLogin];
        frnd.Login = frndLogin;
        frnd.Type = FT_Friend;
        frnd.AuthStatus = AS_WaitingAuthorization;
        frnd.PublicKey = pubKey;
        frnd.ServerPublicKey = serverPubKey;
    } else {
        TFriendInfo& frnd = frndIt->second;
        if (frnd.AuthStatus == AS_UnAuthorized) {
            frnd.AuthStatus = AS_Authorized;
            frnd.PublicKey = pubKey;
            frnd.ServerPublicKey = serverPubKey;
            frnd.NeedOfflineKey = true;
        }
    }
    SyncClientInfo(*info);
}

void TServer::OnFriendOfflineKeyRequest(const string& login,
                                        const string& frndLogin,
                                        const string& offlineKey,
                                        const string& offlineKeySignature)
{
    boost::optional<TClientInfo> info = ClientInfoStorage->Get(login);
    if (!info.is_initialized()) {
        cerr << "onFriendOfflineKeyRequest error: client not exists\n"; // todo: replace with somethin normal
        return;
    }
    TFriendList& friends = info->Friends;
    auto frndIt = friends.find(frndLogin);
    if (frndIt != friends.end()) {
        TFriendInfo& frnd = friends[frndLogin];
        if (!frnd.OfflineKey.empty()) {
            cerr << "onFriendOfflineKeyRequest error: trying to set not-empty offline key\n";
            return;
        }
        frnd.OfflineKey = offlineKey;
        frnd.OfflineKeySignature = offlineKeySignature;
    } else {
        cerr << "onFriendOfflineKeyRequest error: client not exists\n";
        return;
    }
    SyncClientInfo(*info);
}

void TServer::OnOfflineMessageReceived(const string& login,
                                       const string& friendLogin,
                                       const string& message)
{
    boost::optional<TClientInfo> info = ClientInfoStorage->Get(login);
    if (!info.is_initialized()) {
        cerr << "onOfflineMessageReceived error: client not exists\n"; // todo: replace with somethin normal
        return;
    }
    TFriendList& friends = info->Friends;
    auto frndIt = friends.find(friendLogin);
    if (frndIt != friends.end()) {
        TFriendInfo& frnd = friends[friendLogin];
        if (frnd.AuthStatus == AS_Authorized) {
            MessageStorage->Put(login, "i" + Serialize(friendLogin) + Serialize(message), Now()); // store incoming message
        } else {
            cerr << "onOfflineMessageReceived error: friend not exists\n";
            return;
        }
    } else {
        cerr << "onOfflineMessageReceived error: friend not exists\n";
        return;
    }
    SyncClientInfo(*info);
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

    TStringVector messages = MessageStorage->GetMessages(login, from, to);
    if (messages.size() == 0) {
        return; // no sync required
    }
    TClientSyncPacket packet;
    for (size_t i = 0; i < messages.size(); ++i) {
        assert(messages[i].size() > 1);
        TDirectionedMessage* currMessage = packet.add_encryptedmessages();
        string buffer = messages[i].substr(1);
        bool isIncoming = messages[i][0] == 'i';
        string friendLogin;
        string message;
        Deserialize(buffer, friendLogin);
        Deserialize(buffer, message);
        assert(!message.empty());
        assert(!friendLogin.empty());
        currMessage->set_incoming(isIncoming);
        currMessage->set_login(friendLogin);
        currMessage->set_message(message);
    }
    packet.set_from(from.MicroSeconds());
    packet.set_to(to.MicroSeconds());

    string data(1, (ui8)SP_SyncMessages);
    data += packet.SerializeAsString();
    data = Serialize(EncryptSymmetrical(client->SessionKey, Compress(data)));

    Server->Send(data, client->Address);
}

// sends all new messages and friend requests to given client, if it is connected
void TServer::SyncNewMessages(const string& login) {
    auto clientIt = ClientsByLogin.find(login);
    if (clientIt == ClientsByLogin.end()) {
        return;
    }
    TClientRef client = clientIt->second;
    TDuration now = Now();
    SyncMessages(login, client->SessionLastSync, now);
    client->SessionLastSync = now;
}

void TServer::SyncClientInfo(const TClientInfo& info) {
    ClientInfoStorage->Put(info);
    auto cliIt = ClientsByLogin.find(info.Login);
    if (cliIt != ClientsByLogin.end()) {
        // todo: sync only diff between previous and current
        TClientRef& client = cliIt->second;
        client->Info = info;
        TClientSyncInfoPacket packet;
        for (auto it = info.Friends.begin(); it != info.Friends.end(); ++it) {
            const TFriendInfo& frndInfo = it->second;
            TSyncFriend* frnd = packet.add_friends();
            frnd->set_login(frndInfo.Login);
            frnd->set_status(frndInfo.AuthStatus);
            frnd->set_encryptedkey(frndInfo.EncryptedKey);
            frnd->set_publickey(frndInfo.PublicKey);
            frnd->set_serverpublickey(frndInfo.ServerPublicKey);
            if (!frndInfo.OfflineKey.empty()) {
                assert(!frndInfo.OfflineKeySignature.empty() && "missing signature for offline key");
                frnd->set_offlinekey(frndInfo.OfflineKey);
                frnd->set_offlinekeysignature(frndInfo.OfflineKeySignature);
            }
            if (frndInfo.NeedOfflineKey) {
                frnd->set_needofflinekey(frndInfo.NeedOfflineKey);
            }
        }

        assert(!client->SessionKey.empty() && "client must have session key");
        string data(1, (ui8)SP_SyncInfo);
        data += packet.SerializeAsString();
        data = Serialize(EncryptSymmetrical(client->SessionKey, Compress(data)));

        Server->Send(data, client->Address);
    }
}

TPartnerServer::TPartnerServer(const string& host, TPartnerDataCallback& onDataReceived)
    : OnDataReceived(onDataReceived)
{
    // todo: do this in async mode
    std::vector<TNetworkAddressRef> addresses = GetConnectionAddresses(host);
    NUdt::TClientConfig clientConfig;
    clientConfig.ConnectionCallback = std::bind(&TPartnerServer::OnConnected, this, _1);
    //clientConfig.DataReceivedCallback =
    Client.reset(new NUdt::TClient(clientConfig));
    Client->Connect(*addresses[0], false);                // todo: try different addresses
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
