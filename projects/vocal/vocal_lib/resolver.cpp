#include <iostream>
#include <library/resolver/resolver.h>

#include "resolver.h"

using namespace std;

namespace NVocal {

struct TShardedNetworkAddress: public TNetworkAddress {
    size_t Shard;
};

vector<TNetworkAddressRef> GetConnectionAddresses(const string& login, const string& host) {
    vector<TNetworkAddressRef> addresses = GetConnectionAddresses(host);
    vector<TNetworkAddressRef> result;
    for (size_t i = 0; i < addresses.size(); ++i) {
        // todo: filter by user shard
        result.push_back(addresses[i]);
    }
    return result;
}

vector<TNetworkAddressRef> GetConnectionAddresses(const string& host) {
    string queryHost = "_vocal._udp." + host;
    vector<NResolver::TSrvRecord> srvRecords = NResolver::GetSrvRecords(queryHost);
    vector<TNetworkAddressRef> addresses;
    for (size_t i = 0; i < srvRecords.size(); ++i) {
        // todo: add record as sharded network address
        TNetworkAddressRef address = make_shared<TNetworkAddress>(srvRecords[i].Host, srvRecords[i].Port);
        addresses.push_back(address);
    }
    return addresses;
}

} // NVocal
