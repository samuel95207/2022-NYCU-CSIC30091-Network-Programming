#include <arpa/inet.h>

#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <string>



#ifndef _SOCKS_RESPONSE_H_
#define _SOCKS_RESPONSE_H_
#include "Socks4aResponse.h"
#endif

using namespace std;

Socks4aResponse::Socks4aResponse(bool success, uint16_t dstPort, uint32_t dstIp) : dstPort(dstPort), dstIp(dstIp) {
    VN = (byte)0;
    CD = success ? (byte)90 : (byte)91;
}

Socks4aResponse::Socks4aResponse(bool success) {
    VN = (byte)0;
    CD = success ? (byte)90 : (byte)91;
    dstPort = 0;
    dstIp = 0;
}

Socks4aResponse::Socks4aResponse() {}


void Socks4aResponse::print() {
    cout << "VN: " << (int)VN << endl;
    cout << "CD: " << getCommandString() << endl;
    cout << "dstPort: " << dstPort << endl;
    cout << "dstIp: " << getDstIpString() << endl;
}

bool Socks4aResponse::isAccept() { return (CD == (byte)90); }

string Socks4aResponse::getDstIpString() {
    in_addr netAddr;
    netAddr.s_addr = dstIp;
    return string(inet_ntoa(netAddr));
}

string Socks4aResponse::getCommandString() {
    if (CD == (byte)1) {
        return "CONNECT";
    } else if (CD == (byte)2) {
        return "BIND";
    }
    return "";
}

string Socks4aResponse::getAcceptString() { return isAccept() ? "Accept" : "Reject"; }