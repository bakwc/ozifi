
#include <iostream>
#include <memory>

#include <library/http_fetcher/fetcher.h>
#include <library/kwstorage/leveldb.h>
#include <utils/settings.h>

using namespace std;

class THabrFetcher {
public:
    THabrFetcher(const string& storageDir) {
        NKwStorage::TLevelDbStorageOptions options;
        options.DirectoryName = storageDir;
        Storage.reset(NKwStorage::CreateLevelDbStorage(options));
    }

    void Run(size_t idFrom, size_t idTo) {
        cout << "started\n";
        for(size_t i = idFrom; i < idTo; ++i) {
            string key = ToString(i);
            if (Storage->Exists(key)) {
                continue;
            }
            string url = "http://habrahabr.ru/post/" + key + "/";
            optional<string> data = NHttpFetcher::FetchUrl(url, chrono::seconds(20));
            if (data.is_initialized()) {
                Storage->Put(key, *data);
                cout << 100.0 * (i - idFrom) / (idTo - idFrom) << "% (" << key << ") - ok\n";
            } else {
                cout << 100.0 * (i - idFrom) / (idTo - idFrom) << "% (" << key << ") - fail\n";
            }
        }
        cout << "completed\n";
    }
private:
    unique_ptr<NKwStorage::TKwStorage> Storage;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "usage: ./habr_fetcher habr_fetcher.conf";
        return 42;
    }

    try {
        USettings settings;
        settings.Load(argv[1]);
        string storageDir = settings.GetParameter("storage_dir");
        size_t idFrom = settings.GetParameter("id_from");
        size_t idTo = settings.GetParameter("id_to");
        THabrFetcher fetcher(storageDir);
        fetcher.Run(idFrom, idTo);
    } catch (const exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 42;
    }
    return 0;
}
