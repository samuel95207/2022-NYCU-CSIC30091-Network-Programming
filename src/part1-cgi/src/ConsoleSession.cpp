#include <boost/asio.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
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


ConsoleSession::ConsoleSession(boost::asio::io_service& io_service) : socket(io_service), resolver(io_service) {}

void ConsoleSession::start(int idIn, string hostIn, int portIn, string filenameIn, HttpRequest requestIn) {
    id = idIn;
    host = hostIn;
    port = portIn;
    filename = filenameIn;
    request = requestIn;

    cout << "id = " << id << "<br/>";

    // doRead();
}

void ConsoleSession::doRead() {
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

void ConsoleSession::doWrite() {
    // auto self(shared_from_this());
    // std::strcpy(data, "HTTP/1.1 200 OK\r\n");
    // boost::asio::async_write(socket, boost::asio::buffer(data, strlen(data)),
    //                          [this, self](boost::system::error_code errorCode, std::size_t length) {
    //                              if (errorCode) {
    //                                  cerr << "Write Error! " << errorCode << endl;
    //                                  return;
    //                              }

    //                              pid_t pid;
    //                              do {
    //                                  pid = fork();
    //                              } while (pid < 0);

    //                              if (pid > 0) {
    //                                  // Parent Process
    //                                  socket.close();
    //                              } else {
    //                                  dup2(socket.native_handle(), fileno(stdin));
    //                                  dup2(socket.native_handle(), fileno(stdout));
    //                                  dup2(socket.native_handle(), fileno(stderr));
    //                                  socket.close();

    //                                  string cgiPath = "." + request.uriOnly;
    //                                  if (execlp(cgiPath.c_str(), cgiPath.c_str(), NULL) < 0) {
    //                                      std::cerr << "Content-type:text/html\r\n\r\n<h1>Error! CGI not exist</h1>";
    //                                  }
    //                              }
    //                          });
}


void ConsoleSession::recvRequest(string rawRequest) {
    cout << rawRequest << endl;
    request = HttpRequest::parse(rawRequest);

    request.print();
    cout << "REMOTE_ADDR = " << socket.remote_endpoint().address().to_string().c_str() << endl;
    cout << "REMOTE_PORT = " << to_string(socket.remote_endpoint().port()).c_str() << endl;
}
