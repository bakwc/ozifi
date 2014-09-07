#pragma once

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

#include <boost/optional.hpp>

#include <contrib/mongoose/mongoose.h>

using namespace std;
using namespace boost;

namespace NHttpServer {

struct TSettings {
    TSettings(unsigned short port);
    vector<unsigned short> Ports;
    size_t Threads;
    size_t StackSize;
    const char** PrepareSettings();
private:
    vector<string> Options;
    vector<const char*> COptions;
};

typedef unordered_map<string, string> THeaders;

struct TRequest {
    string Method;
    string URI;
    string HttpVersion;
    string Query;
    string PostData;
    string User;
    long IP;
    unsigned short Port;
    bool SSL;
    THeaders Headers;
    optional<string> GetParam(const string& name) const;
};

struct TResponse {
    TResponse();
    string Data;
    int Code;
    string ContentType();
    void ContentType(const string &value);
    string PrepareHeaders();
    THeaders Headers;
};

typedef std::function<optional<TResponse>(const TRequest& request)> TRequestHandler;
typedef unordered_map<string, TRequestHandler> TRequestHandlers;

class THttpServer {
public:
    THttpServer(const TSettings& settings);
    ~THttpServer();
    void HandleAction(const string& action, TRequestHandler handler);
    void HandleActionDefault(TRequestHandler handler);
private:
    static int ProcessRequest(struct mg_connection* conn);
private:
    struct mg_context *Ctx;
    struct mg_callbacks Callbacks;
    TRequestHandlers Handlers;
    TRequestHandler DefaultHandler;
    TSettings Settings;
};

} // NHttpServer
