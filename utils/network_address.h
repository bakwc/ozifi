#pragma once

#include <string>
#include <memory>
#include <boost/asio/ip/address.hpp>
#include <utils/cast.h>

#include "types.h"

struct TNetworkAddress {
    TNetworkAddress()
        : Port(0)
    {
        Addr.sin_family = AF_INET;
        Addr.sin_port = 0;
        Addr.sin_addr.s_addr = INADDR_ANY;
        memset(&(Addr.sin_zero), '\0', 8);
    }
    TNetworkAddress(ui32 host, ui16 port) {
        boost::asio::ip::address_v4::bytes_type bytes;
        memcpy(bytes.data(), &host, 4);
        boost::asio::ip::address_v4 addr(bytes);
        Address = boost::asio::ip::address(addr);
        Port = port;
        Addr.sin_family = AF_INET;
        Addr.sin_port = htons(port);
        Addr.sin_addr.s_addr = host;
        memset(&(Addr.sin_zero), '\0', 8);
    }
    inline bool operator==(const TNetworkAddress& other) const {
        return Address == other.Address && Port == other.Port;
    }
    inline const sockaddr* Sockaddr() const {
        return (sockaddr*)(&Addr);
    }
    inline size_t SockaddrLength() const {
        return sizeof(Addr);
    }
    inline std::string ToString() const {
        return Address.to_string() + ":" + ::ToString(Port);
    }
    boost::asio::ip::address Address;
    ui16 Port;
    sockaddr_in Addr;
};

typedef std::shared_ptr<TNetworkAddress> TNetworkAddressRef;

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
