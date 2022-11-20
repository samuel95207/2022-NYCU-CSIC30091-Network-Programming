#include <boost/asio.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <utility>



#ifndef _HTTP_SESSION_H_
#define _HTTP_SESSION_H_
#include "HttpSession.h"
#endif


#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_
#include "HttpRequest.h"
#endif

#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#include "ConsoleCgi/Console.h"
#endif

#ifndef _PANEL_H_
#define _PANEL_H_
#include "PanelCgi/Panel.h"
#endif

using namespace std;
using boost::asio::ip::tcp;


HttpSession::HttpSession(tcp::socket socket) : socket(std::move(socket)) {}

void HttpSession::start() { doRead(); }

void HttpSession::doRead() {
    auto self(shared_from_this());
    socket.async_read_some(boost::asio::buffer(data, BUF_SIZE),
                           [this, self](boost::system::error_code errorCode, size_t length) {
                               if (errorCode) {
                                   return;
                               }
                               string rawRequest = string(data);
                               recvRequest(rawRequest);
                               doWrite();
                           });
}

void HttpSession::doWrite() {
    auto self(shared_from_this());
    std::strcpy(data, "HTTP/1.1 200 OK\r\n");
    boost::asio::async_write(socket, boost::asio::buffer(data, strlen(data)),
                             [this, self](boost::system::error_code errorCode, std::size_t length) {
                                 if (errorCode) {
                                     cerr << "Write Error! " << errorCode << endl;
                                     return;
                                 }

                                 if (request.uriOnly == "/panel.cgi") {
                                     make_shared<Panel>(std::move(socket))->start(request);
                                 } else if (request.uriOnly == "/console.cgi") {
                                     make_shared<Console>(std::move(socket))->start(request);
                                 } else {
                                     doWrite(
                                         "HTTP/1.1 404 Not Found\r\n"
                                         "Content-type:text/html\r\n\r\n"
                                         "<h1>404 Not Found!</h1>"
                                         "<h2>CGI does not exist.</h2>");
                                 }
                             });
}

void HttpSession::doWrite(string value) {
    auto self(shared_from_this());
    std::strcpy(data, value.c_str());
    boost::asio::async_write(socket, boost::asio::buffer(data, strlen(data)),
                             [this, self](boost::system::error_code errorCode, std::size_t length) {
                                 if (errorCode) {
                                     cerr << "Write Error! " << errorCode << endl;
                                     return;
                                 }
                             });
}



void HttpSession::recvRequest(string rawRequest) {
    cout << rawRequest << endl;
    request = HttpRequest::parse(rawRequest);

    // request.print();
    // cout << "REMOTE_ADDR = " << socket.remote_endpoint().address().to_string().c_str() << endl;
    // cout << "REMOTE_PORT = " << to_string(socket.remote_endpoint().port()).c_str() << endl;
}