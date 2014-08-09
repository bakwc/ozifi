#include <thread>

#include <projects/vocal/vocal_lib/resolver.h>
#include <projects/vocal/vocal_lib/utils.h>
#include <projects/vocal/vocal_lib/compress.h>
#include <projects/vocal/vocal_lib/crypto.h>
#include <projects/vocal/vocal_lib/serializer.h>
#include <projects/vocal/vocal_lib/nat_pmp.h>
#include <projects/vocal/vocal_lib/vocal.pb.h>

#include "friend.h"
#include "client.h"

using namespace std;
using namespace std::placeholders;

namespace NVocal {

TFriend::TFriend(TClient* client)
    : Status(FS_Unauthorized)
    , ConnectionStatus(COS_Offline)
	, ToDelete(false)
	, SelfAuthorized(false)
	, FriendAuthorized(false)
	, AcceptingConnection(false)
    , Client(client)
    , CallStatus(CAS_NotCalling)
{
}

TFriend::TFriend(TClient* client, const string& login, EFriendStatus status)
    : FullLogin(login)
    , Status(status)
    , ToDelete(false)
    , ConnectionStatus(COS_Offline)
    , Client(client)
    , CallStatus(CAS_NotCalling)
{
}

TFriend::~TFriend() {
    Status = FS_Offline;
}

void TFriend::InitUdtClient() {
    NUdt::TClientConfig config;
    config.ConnectionCallback = std::bind(&TFriend::OnConnected, this, _1);
    config.DataReceivedCallback = std::bind(&TFriend::OnDataReceived, this, _1);
    config.ConnectionLostCallback = std::bind(&TFriend::OnDisconnected, this);
    UdtClient.reset(new NUdt::TClient(config));
}

bool TFriend::OnClientConnected(const TNetworkAddress& addr) {
    if (ConnectionStatus != COS_WaitingFriendConnection) {
        return false;
    }
    FriendAddress = addr;
    ConnectionStatus = COS_AcceptedConnection;
    return true;
}

void TFriend::OnConnectionEstablished() {
    SelfAuthorized = false;
    FriendAuthorized = false;
    RandomSequence = GenerateRandomSequence();
    SendSerialized(RandomSequence, FPT_RandomSequence);
}

void TFriend::SendMessage(const TMessage& message) {
    TMessagePacket messagePacket;
    messagePacket.set_text(message.Text);
    messagePacket.set_from(message.From);
    messagePacket.set_to(message.To);
    messagePacket.set_time(message.Time.GetValue());
    string serializedMessage = messagePacket.SerializeAsString();
    if (ConnectionStatus == COS_Connected) { // send direct message only if friend is online
        SendEncrypted(serializedMessage, FPT_Message);
    }
    assert(!FriendOfflineKey.empty() && "missing friend offline key");
    // always send offline message
    Client->SendOfflineMessage(FullLogin, EncryptSymmetrical(FriendOfflineKey, serializedMessage));
}

void TFriend::SendEncrypted(const TBuffer& data, EFriendPacketType friendPacketType) {
    assert(!FriendSessionKey.empty() && "friend session key missing");
    string newData(1, static_cast<ui8>(friendPacketType));
    newData += data.ToString();
    SendSerialized(EncryptSymmetrical(FriendSessionKey, Compress(newData)), FPT_Encrypted);
}

void TFriend::SendSerialized(const TBuffer& data, EFriendPacketType friendPacketType) {
    string serializedData;
    serializedData.reserve(data.Size() + 1);
    serializedData.resize(1);
    serializedData[0] = (char)friendPacketType;
    serializedData += data.ToString();
    SendRaw(Serialize(serializedData));
}

void TFriend::SendRaw(const TBuffer& data) {
    if (ConnectionStatus == COS_Connected) {
        UdtClient->Send(data);
    } else if (ConnectionStatus == COS_AcceptedConnection) {
        assert((bool)UdtServer && "Udt server not initialized");
        UdtServer->Send(data, FriendAddress);
    } else {
        throw UException("not connected to friend");
    }
}

void TFriend::UpdateOnlineStatus() {
    if (SelfAuthorized && FriendAuthorized && !IsOnline(Status)) {
        Status = FS_Available;
        Client->OnFriendStatusChanged(shared_from_this());
    }
}

const string& TFriend::GetLogin() {
    return FullLogin;
}

const string& TFriend::GetName() {
    return Name;
}

EFriendStatus TFriend::GetStatus() {
    return Status;
}

const string& TFriend::GetStatusMessage() {
    assert(false && "unimplemented");
}

std::vector<TMessage> TFriend::GetHistory() {
    assert(false && "unimplemented");
}

void TFriend::SendMessage(const std::string& text) {
    TMessage message;
    message.Text = text;
    message.From = Client->GetFullLogin();
    message.To = FullLogin;
    message.Time = Client->GetTime();
    OnMessageReceived(message);
    SendMessage(message);
}

void TFriend::SendFile(const std::string& name,
              size_t size,
              TDataRequireCallback fileDataCallback)
{
    assert(false && "unimplemented");
}

void TFriend::StartCall(bool videoEnabled) {
    if (!Connected()) {
        throw UException("trying to call when not connected");
    }
    if (CallStatus == CAS_NotCalling) {
        SendEncrypted("", FPT_CallRequest);
        CallStatus = CAS_WaitingFriendConfirm;
        VideoEnabled = videoEnabled;
    } else if (CallStatus == CAS_WaitingSelfConfirm) {
        SendEncrypted("", FPT_CallRequest);
        CallStatus = CAS_Established;
        Client->OnCallStatusChanged(shared_from_this());
    }
}

ECallStatus TFriend::GetCallStatus() {
    return CallStatus;
}

void TFriend::EnableVideo() {
    VideoEnabled = true;
}

void TFriend::DisableVideo() {
    VideoEnabled = false;
}

void TFriend::FinishCall() {
    if (CallStatus != CAS_NotCalling) {
        SendEncrypted("", FPT_CallDrop);
        CallStatus = CAS_NotCalling;
        Client->OnCallStatusChanged(shared_from_this());
    }
}

bool TFriend::Connected() {
    return ConnectionStatus == COS_Connected || ConnectionStatus == COS_AcceptedConnection;
}

// todo: don't try to connect very often
void TFriend::Connect() {
    if (Status != FS_Offline || ConnectionStatus != COS_Offline) {
        return;
    }
    assert(!PublicKey.empty() && "no public key for friend");
    assert(!ServerPublicKey.empty() && "no server key for friend");
    assert(Client && "client is not set");

    if (Client->HasNatPmp()) {
        TNatPmp& natPmp = Client->GetNatPmp();
        PublicAddress = natPmp.GetPublicAddress();
        try {
            natPmp.ForwardRandomPort(LocalPort, PublicPort);
        } catch (const UException&) {
            LocalPort = 0;
            PublicPort = 0;
            cerr << "warning: failed to forward ports with natPmp\n";
        }
    }

    InitUdtClient();

    pair<string, string> loginHost = GetLoginHost(FullLogin);
    std::vector<TNetworkAddressRef> addresses = GetConnectionAddresses(loginHost.first, loginHost.second);
    if (addresses.size() == 0) {
        throw UException("no address found for host");
    }
    // todo: random address selection
    ConnectionStatus = COS_ConnectingToServer;
    AcceptingConnection = false;
    UdtClient->Connect(*addresses[0], false);
}

void TFriend::ConnectAccept() {
    assert(!PublicKey.empty() && "no public key for friend");
    assert(!ServerPublicKey.empty() && "no server key for friend");

    InitUdtClient();

    std::vector<TNetworkAddressRef> addresses = GetConnectionAddresses(Client->State.login(), Client->State.host());
    if (addresses.size() == 0) {
        throw UException("no address found for host");
    }
    // todo: random address selection
    ConnectionStatus = COS_ConnectingToServer;
    AcceptingConnection = true;
    UdtClient->Connect(*addresses[0]);
}

void TFriend::OnConnected(bool success) {
    if (!success) {
        ConnectionStatus = COS_Offline;
        return;
    }
    if (ConnectionStatus == COS_ConnectingToServer) {
        string data;
        data.push_back((char)RT_HelpConnect);
        UdtClient->Send(data);
    } else if (ConnectionStatus == COS_ConnectingToFriend) {
        ConnectionStatus = COS_Connected;
        OnConnectionEstablished();
    }
}

void TFriend::OnDataReceived(const TBuffer& data) {
    try {
        Buffer += data.ToString();
        string packetStr;
        while (Deserialize(Buffer, packetStr)) {
            switch (ConnectionStatus) {
            case COS_ConnectingToServer: {
                packetStr = Decompress(packetStr);
                TServerAuthorizeRequest packet;
                if (!packet.ParseFromString(packetStr)) {
                    cerr << "warning: failed to deserialize server authroize request\n";
                    ForceDisconnect();
                    return;
                }
                if (!CheckSignature(ServerPublicKey, packet.randomsequence(), packet.signature())) {
                    cerr << "warning: failed to check server signature\n";
                    ForceDisconnect();
                    return;
                }
                TClientConnectHelpAuthorizeRequest request;
                assert(!FullLogin.empty() && "friend has no login");
                assert(Client != nullptr && "Client is NULL");
                request.set_login(Client->GetFullLogin());
                request.set_friendlogin(FullLogin);
                string hash = Hash(packet.randomsequence());
                request.set_randomsequencehash(hash);
                request.set_randomsequencehashsignature(Sign(Client->GetPrivateKey(), hash));
                request.set_acceptingconnection(AcceptingConnection);
                if (LocalPort != 0) {
                    TNetworkAddress address = PublicAddress;
                    address.Port = PublicPort;
                    request.set_publicaddress(address.ToString());
                }
                string response = Compress(request.SerializeAsString());
                response = Serialize(EncryptAsymmetrical(ServerPublicKey, response));
                ConnectionStatus = COS_WaitingFriendAddress;
                UdtClient->Send(response);
            } break;
            case COS_WaitingFriendAddress: {
                assert(Client != nullptr && "Client is NULL");
                packetStr = Decompress(DecryptAsymmetrical(Client->GetPrivateKey(), packetStr));
                TServerConnectHelpRequest connectHelp;
                if (!connectHelp.ParseFromString(packetStr)) {
                    cerr << "error: failed to deserialize connect help request\n";
                    return;
                }
                // todo: free nat-pmp port if not used
                switch (connectHelp.connectiontype()) {
                case CTP_Offline: {
                    ForceDisconnect();
                    return;
                } break;
                case CTP_NatTraversal: {
                    ConnectThrowNat(TNetworkAddress(connectHelp.address()), LocalPort);
                } break;
                case CTP_Listen: {
                    UdtClient->Disconnect();
                    if (!UdtServer || UdtServer->GetPort() != LocalPort) {
                        NUdt::TServerConfig config;
                        config.NewConnectionCallback = bind(&TFriend::OnClientConnected, this, _1);
                        config.DataReceivedCallback = bind(&TFriend::OnDataReceived, this, _1);
                        config.ConnectionAcceptedCallback = std::bind(&TFriend::OnConnectionEstablished, this);
                        config.ConnectionLostCallback = std::bind(&TFriend::OnDisconnected, this);
                        config.Port = LocalPort;
                        UdtServer.reset(new NUdt::TServer(config));
                    }
                    ConnectionStatus = COS_WaitingFriendConnection;
                } break;
                case CTP_Connect: {
                    ConnectionStatus = COS_ConnectingToFriend;
                    UdtClient->Connect(connectHelp.address());
                } break;
                }
            } break;
            case COS_Connected:
            case COS_AcceptedConnection: {
                EFriendPacketType packetType = (EFriendPacketType)packetStr[0];
                packetStr = packetStr.substr(1);
                switch (packetType) {
                case FPT_RandomSequence: {
                    TFriendRandomSequenceConfirm confirm;
                    FriendRandomSequence = packetStr;
                    string randomSeqHash = Hash(FriendRandomSequence + "frnd");
                    confirm.set_randomsequencehash(randomSeqHash);
                    confirm.set_signature(Sign(Client->GetPrivateKey(), randomSeqHash));
                    SelfSessionKey = GenerateKey();
                    confirm.set_sessionkey(SelfSessionKey);
                    SendSerialized(EncryptAsymmetrical(PublicKey, Compress(confirm.SerializeAsString())), FPT_RandomSequenceConfirm);
                } break;
                case FPT_RandomSequenceConfirm: {
                    packetStr = Decompress(DecryptAsymmetrical(Client->GetPrivateKey(), packetStr));
                    TFriendRandomSequenceConfirm confirm;
                    if (!confirm.ParseFromString(packetStr)) {
                        throw UException("failed to deserialized");
                    }
                    string randomSeqHash = Hash(RandomSequence + "frnd");
                    if (randomSeqHash != confirm.randomsequencehash()) {
                        throw UException("hash verification failed");
                    }
                    if (!CheckSignature(PublicKey, randomSeqHash, confirm.signature())) {
                        throw UException("signature verification failed");
                    }
                    FriendSessionKey = confirm.sessionkey();
                    SendEncrypted("", FPT_Authorized);
                    FriendAuthorized = true;
                    UpdateOnlineStatus();
                } break;
                case FPT_Encrypted: {
                    packetStr = Decompress(DecryptSymmetrical(SelfSessionKey, packetStr));
                    packetType = static_cast<EFriendPacketType>(packetStr[0]);
                    packetStr = packetStr.substr(1);
                    switch (packetType) {
                    case FPT_Authorized: {
                        SelfAuthorized = true;
                        UpdateOnlineStatus();
                    } break;
                    case FPT_Message: {
                        TMessagePacket messagePacket;
                        if (!messagePacket.ParseFromString(packetStr)) {
                            throw UException("failed to parse message");
                        }
                        TMessage message;
                        message.From = messagePacket.from();
                        message.To = messagePacket.to();
                        message.Time = TDuration(messagePacket.time());
                        message.Text = messagePacket.text();
                        OnMessageReceived(message);
                    } break;
                    case FPT_CallRequest: {
                        if (CallStatus == CAS_NotCalling) {
                            CallStatus = CAS_WaitingSelfConfirm;
                            Client->OnCallStatusChanged(shared_from_this());
                        } else if (CallStatus == CAS_WaitingFriendConfirm) {
                            CallStatus = CAS_Established;
                            Client->OnCallStatusChanged(shared_from_this());
                        }
                    } break;
                    case FPT_CallDrop: {
                        if (CallStatus != CAS_NotCalling) {
                            CallStatus = CAS_NotCalling;
                            Client->OnCallStatusChanged(shared_from_this());
                        }
                    } break;
                    }
                } break;
                }
            } break;
            }
        }
    } catch (const std::exception& e) {
        cerr << "friend data received error: " << e.what() << "\n";
    }
}

void TFriend::OnDisconnected() {
    if (Status == FS_Offline) {
        return;
    }
    Status = FS_Offline;
    ConnectionStatus = COS_Offline;
    Client->OnFriendStatusChanged(shared_from_this());
}

void TFriend::ForceDisconnect() {
    OnDisconnected();
    UdtClient->Disconnect();
    UdtServer.reset(nullptr);
}

void TFriend::ConnectThrowNat(const TNetworkAddress& address, ui16 localPort) {
    if (Status != FS_Offline ||
            !(ConnectionStatus == COS_Offline ||
              ConnectionStatus == COS_WaitingFriendAddress))
    {
        cerr << "warning: wrong status to connect throw nat\n";
        return;
    }
    ConnectionStatus = COS_ConnectingToFriend;
    UdtClient->Connect(address, localPort, true);
}

void TFriend::OnOfflineMessageReceived(const TBuffer& data, bool isIncoming) {
    string decryptedData = DecryptSymmetrical(isIncoming ? SelfOfflineKey : FriendOfflineKey, data);
    TMessagePacket messagePacket;
    if (!messagePacket.ParseFromString(decryptedData)) {
        cerr << "error: failed to deserialize offline message\n";
        return;
    }
    TMessage message;
    message.From = messagePacket.from();
    message.To = messagePacket.to();
    message.Time = TDuration(messagePacket.time());
    message.Text = messagePacket.text();
    OnMessageReceived(message);
}

void TFriend::OnMessageReceived(const TMessage& message) {
    string signature = message.CalcSignature();
    if (PrevMessages.find(signature) != PrevMessages.end()) {
        return;
    }
    PrevMessages.insert(signature);
    Client->OnMessageReceived(message);
}

} // NVocal
