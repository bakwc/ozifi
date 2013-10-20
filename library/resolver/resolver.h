#pragma once

#include <vector>
#include <string>
#include <utils/types.h>

namespace NResolver {

struct TSrvRecord {
    std::string Host;
    ui16 Port;
    ui16 Priority;
    ui16 Weight;
};

std::vector<TSrvRecord> GetSrvRecords(const std::string& queryHost);

} // NResolver
