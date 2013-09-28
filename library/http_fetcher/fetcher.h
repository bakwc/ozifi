#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include <boost/optional.hpp>

using namespace std;
using namespace boost;

namespace NHttpFetcher {

const size_t MAX_FILE_SIZE = 256 * 1024;

struct TRequest {
    TRequest();
    string Url;
    string UserAgent;
    chrono::milliseconds Timeout;
    optional<string> User;
    optional<string> Password;
};

struct TResponse {
    string RequestUrl;
    string ResolvedUrl;
    size_t Code;
    bool Success;
    string Error;
    string Data;
    string Headers;
    optional<string> ParseCharset();
};

optional<string> FetchUrl(const string& url,
                          chrono::milliseconds timeout = chrono::seconds(10));

TResponse FetchUrl(const TRequest& request);

} // NHttpFetcher
