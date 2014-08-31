#include <iostream>
#include <thread>
#include <library/udt/client.h>
#include <library/udt/server.h>


using namespace std;

string Buffer;

void OnDataReceived(const TBuffer& data, const TNetworkAddress& address) {
    Buffer += data.ToString();
}

int main(int argc, char** argv) {

    NUdt::TServerConfig serverConf;
    serverConf.Port = 9988;
    serverConf.DataReceivedCallback = &OnDataReceived;

    NUdt::TServer server(serverConf);

    NUdt::TClientConfig clientConf;
    NUdt::TClient client(clientConf);
    client.Connect(TNetworkAddress("127.0.0.1", serverConf.Port));

    string sendBuff;

    cerr << "sending\n";
    for (size_t i = 0; i < 16000; ++i) {
        string data;
        for (size_t j = 0; j < 800; ++j) {
            data += to_string(i);
        }
        sendBuff += data;
        client.Send(TBuffer(data));
        this_thread::sleep_for(chrono::milliseconds(4));
    }
    cerr << "done\n";
    this_thread::sleep_for(chrono::milliseconds(500));
    cerr << (Buffer == sendBuff ? "equal" : "not equal") << "\n";

    return 0;
}
