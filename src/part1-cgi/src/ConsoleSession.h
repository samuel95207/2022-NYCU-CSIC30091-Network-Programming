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


class ConsoleSession : public enable_shared_from_this<ConsoleSession> {
    static const int BUF_SIZE = 65536;

    tcp::socket socket;
    tcp::resolver resolver;

    int id;
    string host;
    int port;
    string filename;
    HttpRequest request;

    char data[BUF_SIZE];



   public:
    ConsoleSession(boost::asio::io_service& io_service);

    void start(int idIn, string hostIn, int portIn, string filenameIn, HttpRequest requestIn);

   private:
    void doRead();
    void doWrite();

    void recvRequest(string rawRequest);
};