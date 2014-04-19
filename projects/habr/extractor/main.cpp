
#include <iostream>
#include <memory>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <contrib/json/json.h>
#include <library/html_parser/html_parser.h>
#include <utils/settings.h>
#include <utils/string.h>
#include <utils/iostream.h>

using namespace std;

struct THabrArticle {
    string Title;
    string Content;
    int Score;
    vector<string> Hubs;

    string Serialize() {
        Json::FastWriter writer;
        Json::Value root;
        root["title"] = Title;
        root["content"] = Content;
        root["score"] = Score;
        for (size_t i = 0; i < Hubs.size(); ++i) {
            root["hubs"].append(Hubs[i]);
        }
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
    }

    bool GetFromHtml(const string& key, THabrArticle& article) {
        string fileName = HtmlDir + "/" + key + ".html";
        if (!boost::filesystem::exists(fileName)) {
            return false;
        }

        string html = LoadFile(fileName);

        NHtmlParser::THtmlParser dom(html);
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

        std::vector<NHtmlParser::THtmlNode> hubs = dom["hubs"];
        for (size_t i = 0; i < hubs.size(); ++i) {
            article.Hubs.push_back(hubs[i].Text());
        }
        return true;
    }

    bool HasInCache(const string& key) {
        string fileName = ExtractedDir + "/" + key + ".json";
        return boost::filesystem::exists(fileName);
    }

    void Run(size_t idFrom, size_t idTo) {
        cout << "started\n";
        for(size_t i = idFrom; i < idTo; ++i) {
            string key = ToString(i);

            if (HasInCache(key)) {
                continue;
            }

            THabrArticle article;
            if (!GetFromHtml(key, article)) {
                continue;
            }

            SaveFile(ExtractedDir + "/" + key + ".json", article.Serialize());
        }
    }
private:
    string HtmlDir;
    string ExtractedDir;
};

int main(int argc, char** argv) {
    if (argc != 5) {
        Cerr << "usage: ./habr_extractor html_dir extracted_dir id_from id_to\n";
        return 42;
    }

    try {
        string htmlDir = argv[1];
        string extractedDir = argv[2];
        size_t idFrom = FromString(argv[3]);
        size_t idTo = FromString(argv[4]);
        THabrExtractor extractor(htmlDir, extractedDir);
        extractor.Run(idFrom, idTo);
    } catch (const exception& e) {
        Cerr << "error: " << e.what() << "\n";
        return 42;
    }
    return 0;
}
