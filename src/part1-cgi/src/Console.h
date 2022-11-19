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

class Console {
    boost::asio::io_context io_context;

    HttpRequest request;
    map<int, shared_ptr<ConsoleSession>> sessions;


   public:
    void start();
    string renderCommand(string value);
    string renderResponse(string value);

   private:
    void getCgiEnv();
    void renderHtml();

};