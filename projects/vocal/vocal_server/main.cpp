#include <iostream>
#include <thread>
#include <utils/string.h>
#include <utils/settings.h>


#include <projects/vocal/vocal_server_lib/server.h>

using namespace std;
using namespace NVocal;

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Usage: ./vocal_server config.json\n";
        return 42;
    }
    try {
        TServerConfig config;
        USettings settings;
        settings.Load(argv[1]);
        string hostname = settings.GetParameter("hostname");
        string dataDirectory = settings.GetParameter("data_dir");
        config.Port = settings.GetParameter("port");
        config.AdminPort = settings.GetParameter("admin_port");
        config.Hostname = hostname;
        config.DataDirectory = dataDirectory;
        TServer server(config);
        while (true) {
            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 42;
    }
    return 0;
}
