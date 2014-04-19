
#include <iostream>
#include <memory>

#include <boost/filesystem.hpp>

#include <library/http_fetcher/fetcher.h>
#include <utils/settings.h>
#include <utils/string.h>

using namespace std;

class THabrFetcher {
public:
    THabrFetcher(const string& storageDir)
        : StorageDir(storageDir)
    {
    }
    void Run(size_t idFrom, size_t idTo) {
        cout << "started\n";
        for(size_t i = idFrom; i < idTo; ++i) {
            string key = ToString(i);
            string fileName = StorageDir + "/" + key + ".html";
            if (boost::filesystem::exists(fileName)) {
                continue;
            }
            string url = "http://habrahabr.ru/post/" + key + "/";
            optional<string> data = NHttpFetcher::FetchUrl(url, chrono::seconds(20));
            if (data.is_initialized()) {
                SaveFile(fileName, *data);
                cout << 100.0 * (i - idFrom) / (idTo - idFrom) << "% (" << key << ") - ok\n";
            } else {
                cout << 100.0 * (i - idFrom) / (idTo - idFrom) << "% (" << key << ") - fail\n";
            }
        }
        cout << "completed\n";
    }
private:
    string StorageDir;
};

int main(int argc, char** argv) {
    if (argc != 4) {
        cerr << "usage: ./habr_fetcher storage_dir id_from id_to\n";
        return 42;
    }

    try {
        string storageDir = argv[1];
        size_t idFrom = FromString(argv[2]);
        size_t idTo = FromString(argv[3]);
        THabrFetcher fetcher(storageDir);
        fetcher.Run(idFrom, idTo);
    } catch (const std::exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 42;
    }
    return 0;
}
