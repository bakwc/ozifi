#include <cstring>
#include <iostream>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "server.h"

namespace NHttpServer {

TSettings::TSettings(unsigned short port)
    : Threads(100)
    , StackSize(64000)
{
    Ports.push_back(port);
}

const char** TSettings::PrepareSettings() {
    Options.push_back("listening_ports");
    vector<string> ports;
    for (auto p: Ports) ports.push_back(to_string(p));
    Options.push_back(boost::algorithm::join(ports, ","));
    Options.push_back("num_threads");
    Options.push_back(to_string(Threads));
    Options.push_back("stack_size");
    Options.push_back(to_string(StackSize));
    for (size_t i = 0; i < Options.size(); i++) {
        COptions.push_back(Options[i].c_str());
    }
    COptions.push_back(NULL);
    return COptions.data();
}

THttpServer::THttpServer(const TSettings& settings)
    : Settings(settings)
{
    const char** options = Settings.PrepareSettings();

    assert(*options && "options should not be NULL");

    memset(&Callbacks, 0, sizeof(Callbacks));
    Callbacks.begin_request = ProcessRequest;
    Ctx = mg_start(&Callbacks, this, options);

    if (!Ctx) {
        cerr << "Failed to start server\n";
        _exit(42);
        // todo: throw exception here
    }
}

THttpServer::~THttpServer() {
    mg_stop(Ctx);
}

void THttpServer::HandleAction(const string& action, TRequestHandler handler) {
    Handlers.insert(pair<string, TRequestHandler>(action, handler));
}

void THttpServer::HandleActionDefault(TRequestHandler handler) {
    DefaultHandler = handler;
}

int THttpServer::ProcessRequest(struct mg_connection* conn) {
    struct mg_request_info* requestStruct = mg_get_request_info(conn);
    THttpServer* server = static_cast<THttpServer*>(requestStruct->user_data);
    TRequest request;
    request.Method = requestStruct->request_method;
    request.URI = requestStruct->uri;
    request.HttpVersion = requestStruct->http_version;
    if (requestStruct->query_string) {
        request.Query = requestStruct->query_string;
    }

    if (request.Method == "POST") {
        char post_data[1024];
        int post_data_len = 0;
        do {
            post_data_len = mg_read(conn, post_data, sizeof(post_data));
            request.PostData += string(post_data, post_data_len);
        } while (post_data_len > 0);
    }

    if (requestStruct->remote_user) {
        request.User = requestStruct->remote_user;
    }
    request.IP = requestStruct->remote_ip;
    request.Port = requestStruct->remote_port;
    request.SSL = requestStruct->is_ssl;
    for (size_t i = 0; i < requestStruct->num_headers; i++) {
        request.Headers[requestStruct->http_headers[i].name] =
                requestStruct->http_headers[i].value;
    }

    optional<TResponse> response;
    if (server->Handlers.find(request.URI) != server->Handlers.end()) {
        response = server->Handlers[request.URI](request);
    } else if (server->DefaultHandler) {
        response = server->DefaultHandler(request);
    }

    if (!response.is_initialized()) {
        response = TResponse();
        response->Data = "<h3>Unsupported request: " + request.URI + " </h3>\n";
        response->Code = 404;
        response->ContentType("text/html");
    }

    mg_printf(conn,
              "HTTP/1.1 %d OK\r\n"
              "%s"
              "Content-Length: %d\r\n"
              "\r\n",
              response->Code,
              response->PrepareHeaders().c_str(),
              response->Data.size());

    mg_write(conn, response->Data.c_str(), response->Data.size());

    return 1;
}

// todo: unit-tests here
optional<string> TRequest::GetParam(const string& name) const {
    if (boost::algorithm::starts_with(Query, name + "=")) {
        return Query.substr(name.length() + 1, Query.find('&') - name.length() - 1);
    } else {
        size_t pos = Query.find("&" + name + "=");
        if (pos != string::npos) {
            return Query.substr(pos + name.length() + 2,
                                Query.find('&', pos + 1) - name.length() - 2);
        }
    }

    return optional<string>();
}

TResponse::TResponse()
    : Code(200)
{
    ContentType("text/html");
}

string TResponse::ContentType() {
    return Headers["Content-Type"];
}

void TResponse::ContentType(const string& value) {
    Headers["Content-Type"] = value;
}

string TResponse::PrepareHeaders() {
    string headers;
    for (THeaders::iterator it = Headers.begin(); it != Headers.end(); it++) {
        headers += it->first + ": " + it->second + "\r\n";
    }
    return headers;
}

} // NHttpServer
