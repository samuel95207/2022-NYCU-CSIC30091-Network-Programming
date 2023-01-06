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


void Socks4aResponse::print() const {
    cout << "VN: " << (int)VN << endl;
    cout << "CD: " << getAcceptString() << endl;
    cout << "dstPort: " << dstPort << endl;
    cout << "dstIp: " << getDstIpString() << endl;
}

bool Socks4aResponse::isAccept() const { return (CD == (byte)90); }

string Socks4aResponse::getDstIpString() const {
    in_addr netAddr;
    netAddr.s_addr = dstIp;
    return string(inet_ntoa(netAddr));
}

string Socks4aResponse::getAcceptString() const { return isAccept() ? "Accept" : "Reject"; }

void Socks4aResponse::setAccept(bool success) { CD = success ? (byte)90 : (byte)91; }

void Socks4aResponse::setDstPort(uint16_t port) { dstPort = port; }

void Socks4aResponse::setDstIp(uint32_t ip) { dstIp = ip; }

void Socks4aResponse::setDstIpByString(string ip) {
    setDstIp(boost::asio::ip::address::from_string(ip).to_v4().to_uint());
}

void Socks4aResponse::setHton() {
    dstPort = htons(dstPort);
    dstIp = htonl(dstIp);
}