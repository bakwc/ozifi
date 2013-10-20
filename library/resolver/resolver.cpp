#include <boost/serialization/singleton.hpp>
#include <utils/exception.h>
#include <contrib/c-ares/ares.h>
#include <contrib/c-ares/nameser.h>

#include <iostream>

#include "resolver.h"

using namespace std;

namespace NResolver {

void wait_ares(ares_channel channel) {
    for(;;){
        struct timeval *tvp, tv;
        fd_set read_fds, write_fds;
        int nfds;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        nfds = ares_fds(channel, &read_fds, &write_fds);
        if(nfds == 0){
            break;
        }
        tvp = ares_timeout(channel, NULL, &tv);
        select(nfds, &read_fds, &write_fds, NULL, tvp);
        ares_process(channel, &read_fds, &write_fds);
    }
}

struct TCallbackInfo {
    int Status;
    int Timeouts;
    string Data;
};

void callback(void *arg, int status, int timeouts,
              unsigned char *abuf, int alen)
{
    TCallbackInfo* info = (TCallbackInfo*)arg;
    info->Status = status;
    info->Timeouts = timeouts;
    info->Data = string((const char*)abuf, alen);
}

class TAres: public boost::serialization::singleton<TAres> {
public:
    TAres() {
    #if defined (_WIN32) && ! defined (__CYGWIN__)
      WSADATA p;
      WSAStartup ((2<<8) | 2, &p);
    #endif
        int status = ares_library_init(ARES_LIB_INIT_ALL);
        if (status != ARES_SUCCESS) {
            throw UException(std::string("Failed to init ares: ") + ares_strerror(status));
        }
    }

    vector<TSrvRecord> GetRecords(const string& host) const {
        ares_channel channel;
        int status;
        status = ares_init(&channel);
        if(status != ARES_SUCCESS) {
            throw UException(std::string("Failed to init ares channel: ") + ares_strerror(status));
        }
        TCallbackInfo info;
        ares_query(channel, host.c_str(), ns_c_in, ns_t_srv, callback, &info);
        wait_ares(channel);
        ares_destroy(channel);
        if (info.Status != ARES_SUCCESS) {
            throw UException(std::string("Failed to make ares request: ") + ares_strerror(info.Status));
        }
        struct ares_srv_reply* reply;
        status = ares_parse_srv_reply((const unsigned char*)info.Data.data(), info.Data.length(), &reply);
        if (info.Status != ARES_SUCCESS) {
            throw UException(std::string("Failed to parse response: ") + ares_strerror(status));
        }
        vector<TSrvRecord> records;
        struct ares_srv_reply* next = reply;
        while (next != NULL) {
            TSrvRecord record;
            record.Host = next->host;
            record.Port = next->port;
            record.Priority = next->priority;
            record.Weight = next->weight;
            records.push_back(record);
            next = next->next;
        }
        ares_free_data(reply);
        return records;
    }

    ~TAres() {
        ares_library_cleanup();
    }
};

vector<TSrvRecord> GetSrvRecords(const std::string& queryHost) {
    return TAres::get_const_instance().GetRecords(queryHost);
}

} // NResolver
