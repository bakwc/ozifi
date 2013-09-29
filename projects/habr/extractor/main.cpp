
#include <iostream>
#include <memory>

#include <boost/algorithm/string.hpp>

#include <library/kwstorage/leveldb.h>
#include <library/html_parser/html_parser.h>
#include <utils/settings.h>
#include <utils/string.h>
#include <utils/iostream.h>

using namespace std;

class THabrExtractor {
public:
    THabrExtractor(const string& storageDir, const string& extractedDir) {
        HtmlStorage.reset(NKwStorage::CreateLevelDbStorage(storageDir));
        ExtractedStorage.reset(NKwStorage::CreateLevelDbStorage(extractedDir));
    }

    void Run(size_t idFrom, size_t idTo) {
        cout << "started\n";
        for(size_t i = idFrom; i < idTo; ++i) {
            string key = ToString(i);
            boost::optional<string> html = HtmlStorage->Get(key);
            if (!html.is_initialized()) {
                continue;
            }
            NHtmlParser::THtmlParser dom(*html);
            string title = dom(".post_title").Text();
            if (title.empty()) {
                continue;
            }
            Cout << NormalizeText(title, false) << "\n";
       }
       cout << "completed\n";
    }
private:
    unique_ptr<NKwStorage::TKwStorage> HtmlStorage;
    unique_ptr<NKwStorage::TKwStorage> ExtractedStorage;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        Cerr << "usage: ./habr_extractor habr_extractor.conf\n";
        return 42;
    }

    try {
        USettings settings;
        settings.Load(argv[1]);
        string storageDir = settings.GetParameter("storage_dir");
		string extractedDir = settings.GetParameter("extracted_dir");
        size_t idFrom = settings.GetParameter("id_from");
        size_t idTo = settings.GetParameter("id_to");
        THabrExtractor extractor(storageDir, extractedDir);
        extractor.Run(idFrom, idTo);
    } catch (const exception& e) {
        Cerr << "error: " << e.what() << "\n";
        return 42;
    }
    return 0;
}
