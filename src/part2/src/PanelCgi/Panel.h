#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <utility>


#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_
#include "../HttpRequest.h"
#endif


using namespace std;
using boost::asio::ip::tcp;


class Panel : public enable_shared_from_this<Panel> {
    static map<string, string> htmlEscapeMap;
    static const int BUF_SIZE = 65536;


    tcp::socket socket;
    char data[BUF_SIZE];

    HttpRequest request;


   public:
    Panel(tcp::socket socket);
    void start(const HttpRequest& requestIn);

    void writeSocket(string message);

   private:
    void renderHtml();
};