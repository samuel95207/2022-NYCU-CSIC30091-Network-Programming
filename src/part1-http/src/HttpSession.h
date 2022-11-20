#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>


#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_
#include "HttpRequest.h"
#endif


using namespace std;
using boost::asio::ip::tcp;


class HttpSession : public enable_shared_from_this<HttpSession> {
    static const int BUF_SIZE = 1048576;

    int port;
    tcp::socket socket;
    char data[BUF_SIZE];

    HttpRequest request;


   public:
    HttpSession(tcp::socket socket);
    void start();

   private:
    void doRead();
    void doWrite();

    void recvRequest(string rawRequest);
    void setCgiEnv();
};