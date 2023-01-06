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
    static map<string, string> htmlEscapeMap;

    boost::asio::io_context io_context;

    HttpRequest request;
    map<int, shared_ptr<ConsoleSession>> sessions;


   public:
    void start();

    void renderHtml();
    void renderOutput(int sessionId, Output& output);

   private:
    void getCgiEnv();
};