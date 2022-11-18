#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_
#include "HttpServer.h"
#endif

#ifndef _HTTP_SESSION_H_
#define _HTTP_SESSION_H_
#include "HttpSession.h"
#endif

using namespace std;
using boost::asio::ip::tcp;


HttpServer::HttpServer(boost::asio::io_context& io_context, int port)
    : port(port), acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
    doAccept();
}

void HttpServer::doAccept() {
    acceptor.async_accept([this](boost::system::error_code errorCode, tcp::socket socket) {
        if (!errorCode) {
            make_shared<HttpSession>(std::move(socket))->start();
        }
        doAccept();
    });
}
