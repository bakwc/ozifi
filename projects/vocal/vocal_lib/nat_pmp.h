#pragma once

#include <utils/network_address.h>
#include <contrib/libnatpmp/natpmp.h>
#include <contrib/libnatpmp/getgateway.h>

extern "C" {
    extern int initnatpmp(natpmp_t * p, int forcegw, in_addr_t forcedgw);
    extern int sendpublicaddressrequest(natpmp_t * p);
    extern int readnatpmpresponseorretry(natpmp_t * p, natpmpresp_t * response);
    extern int closenatpmp(natpmp_t * p);
    extern int sendnewportmappingrequest(natpmp_t * p, int protocol, uint16_t privateport, uint16_t publicport, uint32_t lifetime);
    extern int readnatpmpresponse(natpmp_t * p, natpmpresp_t * response);
}
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
