#include "sync.h"

namespace NVocal {

TSynchronizer::TSynchronizer(const TSynchronizerConfig &config)
    : Config(config)
{
}

bool TSynchronizer::CheckHost() {
}

void TSynchronizer::SyncMessage(const std::string& login,
                 const std::string& message)
{
}

void TSynchronizer::SyncClientInfo(const TClientInfo& clientInfo) {
}

void TSynchronizer::UpdatePartnerHosts() {
}

}
