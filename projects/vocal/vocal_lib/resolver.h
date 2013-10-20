#pragma once

#include <vector>
#include <utils/network_address.h>

namespace NVocal {

std::vector<TNetworkAddressRef> GetConnectionAddresses(const std::string& login, const std::string& host);
std::vector<TNetworkAddressRef> GetConnectionAddresses(const std::string& host);

} // NVocal
