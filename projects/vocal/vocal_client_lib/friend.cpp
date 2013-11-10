#include <projects/vocal/vocal_lib/resolver.h>
#include <projects/vocal/vocal_lib/utils.h>
#include <projects/vocal/vocal_lib/compress.h>
#include <projects/vocal/vocal_lib/crypto.h>
#include <projects/vocal/vocal_lib/serializer.h>

#include "friend.h"
#include "client.h"

using namespace std;
using namespace std::placeholders;

namespace NVocal {

TFriend::TFriend()
    : ConnectionStatus(COS_Offline)
{
}

TFriend::TFriend(const string& login, EFriendStatus status)
    : Login(login)
    , Status(status)
    , ToDelete(false)
    , ConnectionStatus(COS_Offline)
{
}

const string& TFriend::GetLogin() {
    return Login;
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

void TFriend::SendMssg(const std::string& message) {
    assert(false && "unimplemented");
}

void TFriend::SendFile(const std::string& name,
              size_t size,
              TDataRequireCallback fileDataCallback)
{
    assert(false && "unimplemented");
}

void TFriend::StartCall(TDataRequireCallback videoDataRequireCallback,
               TDataRequireCallback audioDataRequireCallback,
               TDataCallback audioDataCallback,
               TDataCallback videoDataCallback,
               TBoolCallback partnerVideoStatusCallback,
               bool videoEnabled)
{
    assert(false && "unimplemented");
}

void TFriend::EnableVideo() {
    assert(false && "unimplemented");
}

void TFriend::DisableVideo() {
    assert(false && "unimplemented");
}

void TFriend::FinishCall() {
    assert(false && "unimplemented");
}

// todo: don't try to connect very often
void TFriend::Connect() {
    if (Status != FS_Offline || ConnectionStatus != COS_Offline) {
        return;
    }
    assert(!PublicKey.empty() && "no public key for friend");
    assert(!ServerPublicKey.empty() && "no server key for friend");
    NUdt::TClientConfig config;
    config.ConnectionCallback = std::bind(&TFriend::OnConnected, this, _1);
    config.DataReceivedCallback = std::bind(&TFriend::OnDataReceived, this, _1);
    config.ConnectionLostCallback = std::bind(&TFriend::OnDisconnected, this);
    UdtClient.reset(new NUdt::TClient(config));

    pair<string, string> loginHost = GetLoginHost(Login);
    std::vector<TNetworkAddressRef> addresses = GetConnectionAddresses(loginHost.first, loginHost.second);
    if (addresses.size() == 0) {
        throw UException("no address found for host");
    }
    // todo: random address selection
    ConnectionStatus = COS_ConnectingToServer;
    UdtClient->Connect(*addresses[0], false);
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
        cerr << "ESTABLISHED FRIEND CONNECTION!\n"; // todo: process friend connection
    }
}

void TFriend::OnDataReceived(const TBuffer& data) {
    Buffer += data.ToString();
    string packetStr;
    if (!Deserialize(Buffer, packetStr)) {
        return;
    }

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
        assert(!Login.empty() && "friend has no login");
        request.set_login(Client->GetLogin());
        request.set_friendlogin(Login);
        string hash = Hash(packet.randomsequence());
        request.set_randomsequencehash(hash);
        request.set_randomsequencehashsignature(Sign(Client->GetPrivateKey(), hash));
        string response = Compress(request.SerializeAsString());
        response = Serialize(EncryptAsymmetrical(ServerPublicKey, response));
        ConnectionStatus = COS_WaitingFriendAddress;
        UdtClient->Send(response);
    } break;
    case COS_WaitingFriendAddress: {
        TConnectFriendRequest packet;
        string friendAddress = Decompress(DecryptAsymmetrical(Client->GetPrivateKey(), packetStr));
        if (friendAddress == "offline") {
            ForceDisconnect();
            return;
        }
        ConnectThrowNat(friendAddress);
    } break;
    }
}

void TFriend::OnDisconnected() {
}

void TFriend::ForceDisconnect() {
    Status = FS_Offline;
    ConnectionStatus = COS_Offline;
    UdtClient->Disconnect();
}

void TFriend::ConnectThrowNat(const TNetworkAddress& address) {
    if (Status != FS_Offline ||
            !(ConnectionStatus == COS_Offline ||
              ConnectionStatus == COS_WaitingFriendAddress))
    {
        cerr << "warning: wrong status to connect throw nat\n";
        return;
    }
    ConnectionStatus = COS_ConnectingToFriend;
    UdtClient->Connect(address, true);
}

} // NVocal
