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
    strcpy(data, "HTTP/1.1 200 OK\r\n");
    boost::asio::async_write(
        socket, boost::asio::buffer(data, strlen(data)),
        [this, self](boost::system::error_code errorCode, std::size_t length) {
            if (errorCode) {
                cerr << "HTTP/1.1 500 Internal Server Error\r\n";
                cerr << "Write Error! " << errorCode << endl;
                return;
            }

            pid_t pid;
            do {
                pid = fork();
            } while (pid < 0);

            if (pid > 0) {
                // Parent Process
                socket.close();
            } else {
                setCgiEnv();
                dup2(socket.native_handle(), fileno(stdin));
                dup2(socket.native_handle(), fileno(stdout));
                dup2(socket.native_handle(), fileno(stderr));
                socket.close();

                string cgiPath = "." + request.uriOnly;
                if (execlp(cgiPath.c_str(), cgiPath.c_str(), NULL) < 0) {
                    cerr << "HTTP/1.1 404 Not Found\r\n";
                    cerr << "Content-type:text/html\r\n\r\n<h1>404 Not Found!</h1><h2>CGI does not exist.</h2>";
                    exit(0);
                }
            }
        });
}


void HttpSession::recvRequest(string rawRequest) {
    cout << rawRequest << endl;
    request = HttpRequest::parse(rawRequest);

    request.print();
    cout << "REMOTE_ADDR = " << socket.remote_endpoint().address().to_string().c_str() << endl;
    cout << "REMOTE_PORT = " << to_string(socket.remote_endpoint().port()).c_str() << endl;
}

void HttpSession::setCgiEnv() {
    setenv("REQUEST_METHOD", request.method.c_str(), 1);
    setenv("REQUEST_URI", request.uri.c_str(), 1);
    setenv("QUERY_STRING", request.queryStr.c_str(), 1);
    setenv("SERVER_PROTOCOL", request.version.c_str(), 1);
    setenv("HTTP_HOST", request.host.c_str(), 1);
    setenv("SERVER_ADDR", request.addr.c_str(), 1);
    setenv("SERVER_PORT", to_string(request.port).c_str(), 1);
    setenv("REMOTE_ADDR", socket.remote_endpoint().address().to_string().c_str(), 1);
    setenv("REMOTE_PORT", to_string(socket.remote_endpoint().port()).c_str(), 1);
}
