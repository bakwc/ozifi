#pragma once

#include <string>
#include <vector>
#include <utils/types.h>
#include <utils/network_address.h>

// todo: implement partner hosts synchronization

#include "storage.h"

namespace NVocal {

struct TSynchronizerConfig {
    ui16 Port;
    std::string Host;
    TClientInfoStorage& ClientStorage;
    TMessageStorage& MessageStorage;
};

class TSynchronizer {
public:
    TSynchronizer(const TSynchronizerConfig config);
    bool CheckHost();                                        // checks if srv record is correct
    void SyncMessage(const std::string& login,               // sends given message to partner hosts
                     const std::string& message);
    void SyncClientInfo(const TClientInfo& clientInfo);      // sends client info to partner hosts
private:
    void UpdatePartnerHosts();
    std::vector<TNetworkAddressRef> PartnerHosts;
    TSynchronizerConfig Config;
};

}
