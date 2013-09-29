#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <utils/string.h>
#include <contrib/htmlcxx/html/ParserDom.h>

#include "html_parser.h"
#include "utils.h"

using namespace htmlcxx;
using namespace std;

namespace NHtmlParser {

typedef tree<HTML::Node> TDomTree;
typedef std::unordered_multimap<std::string, TDomTree::iterator> TDomTreeHash;

class THtmlNodeImpl {
public:
    THtmlNodeImpl(const TDomTree::iterator& iterator)
        : Iterator(iterator)
    {
    }
public:
    std::string Text() {
        return Iterator->text();
    }
    void Text(const std::string& text) {
        Iterator->text(text);
    }
private:
    TDomTree::iterator Iterator;
};

THtmlNode::THtmlNode(THtmlNodeImpl* impl)
    : NodeImpl(impl)
{
}

THtmlNode::~THtmlNode() {
}

std::string THtmlNode::Text() const {
    if (NodeImpl) {
        return NodeImpl->Text();
    }
    return std::string();
}

class THtmlParserImpl {
public:
    THtmlParserImpl(const TOptions& options)
        : Options(options)
    {
    }

    void BuildIndex(const TDomTree& tree, const boost::optional<string> recodeCharset) {
        for (TDomTree::iterator element = tree.begin(); element != tree.end(); ++element) {
            HTML::Node& node = *element;
            if (!node.text().empty() && recodeCharset.is_initialized()) {
                node.text(RecodeText(node.text(), *recodeCharset, "UTF-8"));
            }
            if (node.isTag()) {
                node.parseAttributes();
                map<string, string> attributes = node.attributes();
                map<string, string>::iterator it;
                for (it = attributes.begin(); it != attributes.end(); it++) {
                    string key = it->first;
                    string value = it->second;
                    boost::algorithm::to_lower(key);
                    boost::algorithm::to_lower(value);
                    if (key == "class" || key == "id") {
                        TDomTreeHash* hash = key == "class" ? &ElementsByClass : &ElementsById;
                        vector<string> values;
                        boost::algorithm::split(values, value, boost::algorithm::is_any_of(" "));
                        for (size_t i = 0; i < values.size(); i++) {
                            hash->insert(pair<string, TDomTree::iterator>(value, element));
                        }
                    }
                }
            }
        }
    }

    void Parse(const std::string& html) {
        DOM = Parser.parseTree(html);
        string charset;
        boost::optional<string> charsetFrom;
        if (Options.ProcessEncoding == PE_Recomended) {
            if (!GetCharset(DOM.begin(), charset)) {
                charset = Options.Encoding;
            }
        } else if (Options.ProcessEncoding == PE_Forced) {
            charset = Options.Encoding;
        }
        boost::algorithm::to_upper(charset);
        if (Options.ProcessEncoding != PE_None &&
                !charset.empty() &&
                charset != "UTF-8" &&
                charset != "UTF8")
        {
            charsetFrom = charset;
        }
        BuildIndex(DOM.begin(), charsetFrom);
    }

    std::vector<THtmlNode> SearchIn(const TDomTreeHash& hash, const std::string& key) {
        std::pair<TDomTreeHash::const_iterator, TDomTreeHash::const_iterator> range;
        range = hash.equal_range(key);
        std::vector<THtmlNode> results;
        for (TDomTreeHash::const_iterator it = range.first; it != range.second; ++it) {
            results.push_back(THtmlNode(new THtmlNodeImpl(it->second)));
        }
        return results;
    }

    std::vector<THtmlNode> SearchMulti(const std::string& selector) {
        if (boost::algorithm::starts_with(selector, "#")) {
            return SearchIn(ElementsById, selector.substr(1));
        } else if (boost::algorithm::starts_with(selector, ".")) {
            return SearchIn(ElementsByClass, selector.substr(1));
        }
        return std::vector<THtmlNode>();
    }

    THtmlNode SearchSingle(const std::string& selector) {
        std::vector<THtmlNode> results = SearchMulti(selector);
        if (!results.empty()) {
            return results[0];
        }
        return THtmlNode(NULL);
    }
private:
    TOptions Options;
    HTML::ParserDom Parser;
    TDomTree DOM;
    TDomTreeHash ElementsById;
    TDomTreeHash ElementsByClass;
};

THtmlParser::THtmlParser(const TOptions& options)
    : ParserImpl(new THtmlParserImpl(options))
{
}

THtmlParser::THtmlParser(const std::string& html, const TOptions& options)
    : ParserImpl(new THtmlParserImpl(options))
{
    ParserImpl->Parse(html);
}

THtmlParser::~THtmlParser() {
}

void THtmlParser::Parse(const std::string html) {
    ParserImpl->Parse(html);
}

THtmlNode THtmlParser::operator ()(const std::string& selector) {
    return ParserImpl->SearchSingle(selector);
}

std::vector<THtmlNode> THtmlParser::operator[](const std::string& selector) {
    return ParserImpl->SearchMulti(selector);
}

} // NHtmlParser
