#include <projects/vocal/vocal_lib/resolver.h>
#include <projects/vocal/vocal_lib/utils.h>

#include "friend.h"

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
    NUdt::TClientConfig config;
    config.ConnectionCallback = std::bind(&TFriend::OnConnected, this, _1);
    config.DataReceivedCallback = std::bind(&TFriend::OnDataReceived, this, _1);
    config.ConnectionLostCallback = std::bind(&TFriend::OnDisconnected, this);
    Client.reset(new NUdt::TClient(config));

    pair<string, string> loginHost = GetLoginHost(Login);
    std::vector<TNetworkAddressRef> addresses = GetConnectionAddresses(loginHost.first, loginHost.second);
    if (addresses.size() == 0) {
        throw UException("no address found for host");
    }
    // todo: random address selection
    ConnectionStatus = COS_ConnectingToServer;
    Client->Connect(*addresses[0], false);
}

void TFriend::OnConnected(bool success) {
    if (!success) {
        ConnectionStatus = COS_Offline;
        return;
    }

}

void TFriend::OnDataReceived(const TBuffer& data) {
}

void TFriend::OnDisconnected() {
}

} // NVocal
