#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <utility>

#ifndef _CONSOLE_SESSION_H_
#define _CONSOLE_SESSION_H_
#include "ConsoleSession.h"
#endif

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_
#include "HttpRequest.h"
#endif


using namespace std;
using boost::asio::ip::tcp;


class Console : public enable_shared_from_this<Console> {
    static map<string, string> htmlEscapeMap;
    static const int BUF_SIZE = 1048576;


    tcp::socket socket;
    boost::asio::io_context io_context;
    char data[BUF_SIZE];


    HttpRequest request;
    map<int, shared_ptr<ConsoleSession>> sessions;


   public:
    Console(tcp::socket socket);
    void start(const HttpRequest& requestIn);

    void writeSocket(string message);
    void renderHtml();
    string renderCommand(string value);
    string renderResponse(string value);
    string renderError(string value);
};