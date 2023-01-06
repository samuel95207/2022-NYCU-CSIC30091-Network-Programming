#include <arpa/inet.h>

#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <string>



#ifndef _SOCKS_REQUEST_H_
#define _SOCKS_REQUEST_H_
#include "Socks4aRequest.h"
#endif

using namespace std;

Socks4aRequest Socks4aRequest::parse(char* rawRequest) {
    Socks4aRequest request;
    uint16_t dstPort;
    uint32_t dstIp;


    request.VN = (byte)rawRequest[0];
    request.CD = (byte)rawRequest[1];
    memcpy(&(dstPort), &(rawRequest[2]), sizeof(dstPort));
    memcpy(&(dstIp), &(rawRequest[4]), sizeof(dstIp));

    request.dstPort = ntohs(dstPort);
    request.dstIp = ntohl(dstIp);


    request.userId = string(&(rawRequest[8]));


    if (request.needDomainResolve()) {
        request.domainName = string(&(rawRequest[8 + request.userId.length() + 1]));
    }
    return request;
}

void Socks4aRequest::print() const {
    cout << "VN: " << (int)VN << endl;
    cout << "CD: " << getCommandString() << endl;
    cout << "dstPort: " << dstPort << endl;
    cout << "dstIp: " << getDstIpString() << endl;
    cout << "userId: " << userId << endl;
    cout << "domainName: " << domainName << endl;
}

bool Socks4aRequest::needDomainResolve() const { return (dstIp > (uint32_t)0 && dstIp < (uint32_t)256); }


string Socks4aRequest::getDstIpString() const {
    in_addr netAddr;
    netAddr.s_addr = htonl(dstIp);
    return string(inet_ntoa(netAddr));
}

string Socks4aRequest::getCommandString() const {
    if (CD == (byte)1) {
        return "CONNECT";
    } else if (CD == (byte)2) {
        return "BIND";
    }
    return "";
}

void Socks4aRequest::resolveDomainName(boost::asio::ip::tcp::resolver& resolver) {
    if (!needDomainResolve()) {
        return;
    }

    boost::asio::ip::tcp::resolver::query query(domainName, to_string(dstPort));
    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
    dstIp = iter->endpoint().address().to_v4().to_uint();
}

int Socks4aRequest::size() { return 9 + userId.length() + domainName.length() + (domainName.length() != 0); }

char* Socks4aRequest::byteArr() {
    char* result = new char[size()];
    memcpy(result, this, 8);
    memcpy(&result[8], userId.c_str(), userId.length());
    result[8 + userId.length()] = 0;
    memcpy(&result[8 + userId.length() + 1], domainName.c_str(), domainName.length());
    result[size() - 1] = 0;

    return result;
}
