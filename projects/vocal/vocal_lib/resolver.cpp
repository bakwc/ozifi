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
    // todo: filter by user shard
    return result;
}

vector<TNetworkAddressRef> GetConnectionAddresses(const string& host) {
    string queryHost = "_vocal._udp." + host;
    vector<NResolver::TSrvRecord> srvRecords = NResolver::GetSrvRecords(queryHost);
    vector<TNetworkAddressRef> addresses;
    for (size_t i = 0; i < srvRecords.size(); ++i) {
        // todo: add record as sharded network address
    }
    return addresses;
}

} // NVocal
