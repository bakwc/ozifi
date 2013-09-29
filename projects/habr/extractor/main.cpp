
#include <iostream>
#include <memory>
#include <vector>

#include <boost/algorithm/string.hpp>

#include <contrib/json/json.h>
#include <library/kwstorage/leveldb.h>
#include <library/html_parser/html_parser.h>
#include <utils/settings.h>
#include <utils/string.h>
#include <utils/iostream.h>

using namespace std;

struct THabrArticle {
    string Title;
    string Content;
    int Score;
    string Serialize() {
        Json::FastWriter writer;
        Json::Value root;
        root["title"] = Title;
        root["content"] = Content;
        root["score"] = Score;
        return writer.write(root);
    }
    void Parse(const std::string& data) {
        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(data, root)) {
            throw UException("parse error");
        }
        Title = root["title"].asCString();
        Content = root["content"].asCString();
        Score = root["score"].asInt();
    }
};

typedef vector<THabrArticle> THabrArticles;

class THabrExtractor {
public:
    THabrExtractor(const string& storageDir, const string& extractedDir) {
        HtmlStorage.reset(NKwStorage::CreateLevelDbStorage(storageDir));
        ExtractedStorage.reset(NKwStorage::CreateLevelDbStorage(extractedDir));
    }

    bool GetFromHtml(const string& key, THabrArticle& article) {
        boost::optional<string> html = HtmlStorage->Get(key);
        if (!html.is_initialized()) {
            return false;
        }
        NHtmlParser::THtmlParser dom(*html);
        string title = dom(".post_title").Text();
        string text = dom(".content").Text();
        if (title.empty() || text.empty()) {
            return false;
        }

        string rateStr = RecodeText(dom(".infopanel").Find(".score").Text(), "utf-8", "cp1251");
        int rate;
        try {
            rate = FromString(rateStr.substr(1));
        } catch (const UException&) {
            return false;
        }

        if (rateStr[0] != '+') {
            rate = -rate;
        }

        article.Title = title;
        article.Content = text;
        article.Score = rate;
        ExtractedStorage->Put(key, article.Serialize());
        return true;
    }

    bool GetFromCache(const string& key, THabrArticle& article) {
        boost::optional<string> data = ExtractedStorage->Get(key);
        if (!data.is_initialized()) {
            return false;
        }
        article.Parse(*data);
        return true;
    }

    void Run(size_t idFrom, size_t idTo) {
        cout << "started\n";
        float prevPerc = 0;
        for(size_t i = idFrom; i < idTo; ++i) {
            string key = ToString(i);

            THabrArticle article;
            if (!GetFromCache(key, article) && !GetFromHtml(key, article)) {
                continue;
            }

            if (article.Score >= 115) {
                ArticlesPositive.push_back(article);
            } else if (article.Score < 85) {
                ArticlesNegative.push_back(article);
            }
            float currPerc = 100.0 * (i - idFrom) / (idTo - idFrom);
            if (currPerc - prevPerc > 1) {
                cout << currPerc << "% (" << key << ")\n";
                prevPerc = currPerc;
            }
        }
        cout << "positive size: " << ArticlesPositive.size() << "\n";
        cout << "negative size: " << ArticlesNegative.size() << "\n";
    }
private:
    unique_ptr<NKwStorage::TKwStorage> HtmlStorage;
    unique_ptr<NKwStorage::TKwStorage> ExtractedStorage;
    THabrArticles ArticlesPositive;
    THabrArticles ArticlesNegative;
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
