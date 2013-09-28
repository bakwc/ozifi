#include <iostream>
#include <memory>
#include <thread>

#include <library/http_server/server.h>
#include <library/http_fetcher/fetcher.h>
#include <contrib/json/json.h>
#include <utils/string.h>

using namespace std;
using namespace std::placeholders;
using namespace NHttpServer;

class TServer {
public:
    TServer(const string& configFile)
        : Server()
    {
        Json::Reader reader;
        Json::Value root;
        string configData;

        configData = LoadFile(configFile);
        if (!reader.parse(configData, root)) {
            cerr << "ERROR: failed to parse config file '" << configFile << "'\n";
            _exit(42);
            // todo: throw exception here
        }

        ServerPort = root["server_port"].asUInt();
        Threads = root["threads"].asUInt();
    }

    void Run() {
        NHttpServer::TSettings settings(ServerPort);
        settings.Threads = Threads;
        Server.reset(new NHttpServer::THttpServer(settings));
        Server->HandleActionDefault(std::bind(&TServer::ProcessRequest, this, _1));
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    optional<TResponse> ProcessRequest(const TRequest& request) {
        string url;
        auto host = request.Headers.find("Host");
        if (host != request.Headers.end()) {
            url = "http://" + host->second + request.URI;
        }

        TResponse response;
        response.Code = 200;
        if (!url.empty()) {
            cerr << "url: " << url << "\n";
            optional<string> data = NHttpFetcher::FetchUrl(url);
            if (data.is_initialized()) {
                response.Data = *data;
            }
        }
        return response;
    }
private:
    unsigned short ServerPort;
    size_t Threads;
    unique_ptr<THttpServer> Server;
};

int main(int argc, char** argv) {
    setlocale(LC_CTYPE, "en_US.UTF-8");

    if (argc != 2) {
        cerr << "\n  Usage " + string(argv[0]) + " config.json\n\n";
        return 42;
    }

    TServer server(argv[1]);
    server.Run();

    return 0;
}
