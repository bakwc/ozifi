#pragma once

#include <string>
#include <vector>
#include <memory>

#include <boost/optional.hpp>

namespace NHtmlParser {

enum EProcessEncoding {
    PE_None,            // ignore HTML meta encoding, use UTF-8
    PE_Recomended,      // use HTML meta encoding if exists, otherwise use provided encoding
    PE_Forced           // ignore HTML meta encoding, use provided encoding always
};

struct TOptions {
    TOptions()
        : ProcessEncoding(PE_Recomended)
    {
    }
    EProcessEncoding ProcessEncoding;
    std::string Encoding;   // encoding, passed by http header or just known
};

class THtmlNodeImpl;
class THtmlNode {
public:
    THtmlNode(THtmlNodeImpl* impl);
    ~THtmlNode();
    std::string Text(bool trim = true) const;
    THtmlNode Find(const std::string& selector);
    inline bool Exists() {
        return NodeImpl != 0;
    }
private:
    std::shared_ptr<THtmlNodeImpl> NodeImpl;
};

class THtmlParserImpl;
class THtmlParser {
public:
    THtmlParser(const TOptions& options = TOptions());
    THtmlParser(const std::string& html, const TOptions& options = TOptions());
    ~THtmlParser();
    void Parse(const std::string html); // builds html tree, throws exception on failure
    // returns single element by specified selector
    THtmlNode operator ()(const std::string& selector);
    // returns array of elements by specified selector
    std::vector<THtmlNode> operator[](const std::string& selector);
private:
    std::unique_ptr<THtmlParserImpl> ParserImpl;
};

} // NHtmlParser
