#include <utils/exception.h>
#include <utils/cast.h>
#include <library/captcha/captcha.h>
#include <projects/vocal/vocal_lib/resolver.h>
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
{
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
            client->Login = CS_Authorizing;
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
                client->Status = CS_Authorized;
                client->SessionKey = key;
                response = Serialize(EncryptAsymmetrical(clientInfo->PublicKey, Compress(key)));
            }
        } catch (const std::exception& e) {
            cout << "notice:\tclient authorization error: " << e.what() << "\n";
            response = boost::optional<string>();
            disconnectClient = true;
        }
    } break;
    }

    if (response.is_initialized()) {
        Server->Send(TBuffer(response.get()), addr);
    }
    if (disconnectClient) { // check if it waits for data to be send
        Server->DisconnectClient(addr);
    }
}

}
