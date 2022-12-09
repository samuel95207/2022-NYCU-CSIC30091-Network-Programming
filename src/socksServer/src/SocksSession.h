#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>


#ifndef _SOCKS_REQUEST_H_
#define _SOCKS_REQUEST_H_
#include "Socks4aRequest.h"
#endif

#ifndef _SOCKS_RESPONSE_H_
#define _SOCKS_RESPONSE_H_
#include "Socks4aResponse.h"
#endif

#ifndef _FIREWALL_H_
#define _FIREWALL_H_
#include "Firewall.h"
#endif



using namespace std;
using boost::asio::ip::tcp;


class SocksSession : public enable_shared_from_this<SocksSession> {
    static const int BUF_SIZE = 1048576;

    tcp::socket clientSocket;
    tcp::socket serverSocket;
    tcp::acceptor acceptor;
    tcp::resolver resolver;


    char socksRequestBuf[BUF_SIZE];
    char readFromClientBuf[BUF_SIZE];
    char readFromServerBuf[BUF_SIZE];
    char writeToClientBuf[BUF_SIZE];
    char writeToServerBuf[BUF_SIZE];


    int port;
    Socks4aRequest request;
    Socks4aResponse response;

    Firewall firewall;


   public:
    SocksSession(tcp::socket socket, boost::asio::io_service& io_service);
    void start();

   private:
    void readSocks();
    void replySocks();

    void createConnectTunnel();
    void createBindTunnel();

    void readFromClient();
    void readFromServer();
    void writeToClient(std::size_t length);
    void writeToServer(std::size_t length);




    void printSocksInfo();
};