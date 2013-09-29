#include <boost/algorithm/string.hpp>

#include "utils.h"

using namespace htmlcxx;
using namespace std;

string GetTag(tree<HTML::Node>::iterator it) {
    string tag;
    HTML::Node& node = *it;
    if (node.isTag()) {
        tag = node.tagName();
        tag.erase(std::remove(tag.begin(), tag.end(), '\n'), tag.end());
        boost::algorithm::to_lower(tag);
    }
    return tag;
}

bool GetCharset(tree<HTML::Node>::iterator it, string& charset) {
    HTML::Node& node = *it;
    if (node.isTag()) {
        string tag = GetTag(it);
        if (tag == "meta") {
            node.parseAttributes();
            map<string, string> attributes = node.attributes();
            map<string, string>::iterator it;
            bool hasContentType = false;
            string contentType;
            for (it = attributes.begin(); it != attributes.end(); it++) {
                string key = it->first;
                string value = it->second;
                boost::algorithm::to_lower(key);
                boost::algorithm::to_lower(value);
                if (key == "http-equiv" && value == "content-type") {
                    hasContentType = true;
                }
                if (key == "content") {
                    contentType = value;
                }
            }
            if (hasContentType) {
                vector<string> params;
                boost::algorithm::split(params, contentType, boost::algorithm::is_any_of("; "));
                for (size_t i = 0; i < params.size(); i++) {
                    if (boost::algorithm::starts_with(params[i], "charset=")) {
                        charset = params[i].substr(8);
                        return true;
                    }
                }
            }
        }

        if (tag != "html" && !tag.empty()) {
            return false;
        }

        for (tree<HTML::Node>::iterator jt = it.begin(); jt != it.end(); jt++) {
            if (GetCharset(jt, charset)) {
                return true;
            }
        }
    }
    return false;
}
