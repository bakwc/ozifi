#pragma once

#include <string>
#include <boost/asio/ip/address.hpp>

#include "types.h"

struct TNetworkAddress {
    TNetworkAddress(ui32 host, ui16 port) {
        boost::asio::ip::address_v4::bytes_type bytes;
        memcpy(bytes.data(), &host, 4);
        boost::asio::ip::address_v4 addr(bytes);
        Address = boost::asio::ip::address(addr);
        Port = port;
    }
    bool operator==(const TNetworkAddress& other) const {
        return Address == other.Address && Port == other.Port;
    }
    boost::asio::ip::address Address;
    ui16 Port;
};

namespace std {
  template <>
  struct hash<TNetworkAddress>
  {
    std::size_t operator()(const TNetworkAddress& k) const
    {
      using std::size_t;
      using std::hash;
      using std::string;

      std::string key = k.Address.to_string() + to_string(k.Port);
      // todo: improve speed, remove to_string()
      return std::_Hash_impl::hash(key.data(), key.size());
    }
  };
}
