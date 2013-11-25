#pragma once

#include <utils/network_address.h>
#include <contrib/libnatpmp/natpmp.h>

namespace NVocal {

// todo: rewrite it async way

class TNatPmp {
public:
    TNatPmp();
    ~TNatPmp();
    const TNetworkAddress& GetPublicAddress();                  // returns public ip with zero port
    void ForwardRandomPort(ui16& localPort, ui16& publicPort);  // forwards random port
    void CancelForwarding(ui16 localPort);                      // cancel ports forwarding
private:
    natpmp_t NatPmp;
    TNetworkAddress PublicAddress;
};

} // NVocal
