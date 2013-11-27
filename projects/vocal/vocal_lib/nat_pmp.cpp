#include <thread>
#include <utils/exception.h>
#include <stdint.h>
#include "nat_pmp.h"

namespace NVocal {

TNatPmp::TNatPmp() {
    if (initnatpmp(&NatPmp, 0, 0) < 0) {
        throw UException("failed to init nat_pmp");
    }
    if (sendpublicaddressrequest(&NatPmp) < 0) {
        throw UException("failed to send nat_pmp pubaddr request");
    }
    natpmpresp_t response;
    for (size_t attempts = 0; attempts < 30; ++attempts) {  // waiting 3 seconds to connect
        int r = readnatpmpresponseorretry(&NatPmp, &response);
        if(r < 0 && r != NATPMP_TRYAGAIN || attempts == 29) {
            throw UException("failed to get nat_pmp pubaddr");
        } else if (r != NATPMP_TRYAGAIN) {
          ui32 addr = response.pnu.publicaddress.addr.s_addr;
          PublicAddress = TNetworkAddress(addr, 0);
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

TNatPmp::~TNatPmp() {
    closenatpmp(&NatPmp);
}

const TNetworkAddress& TNatPmp::GetPublicAddress() {
    return PublicAddress;
}

void TNatPmp::ForwardRandomPort(ui16& localPort, ui16& publicPort) {
    ui32 reqLocalPort = 1028 + rand() % 40000;
    ui32 reqPublicPort = reqLocalPort;
    if (sendnewportmappingrequest(&NatPmp, NATPMP_PROTOCOL_UDP, reqLocalPort, reqPublicPort, 43200) < 0) { // 12 hours by default
        throw UException("failed to forward port");
    }
    natpmpresp_t response;
    for (size_t attempts = 0; attempts < 30; ++attempts) {  // waiting 3 seconds to forward port
        int r = readnatpmpresponseorretry(&NatPmp, &response);
        if(r < 0 && r != NATPMP_TRYAGAIN || attempts == 29) {
            throw UException("failed to forward port - no response");
        } else if (r != NATPMP_TRYAGAIN) {
          localPort = response.pnu.newportmapping.privateport;
          publicPort = response.pnu.newportmapping.mappedpublicport;
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void TNatPmp::CancelForwarding(ui16 localPort) {
    // todo: implement this
}


} // NVocal
