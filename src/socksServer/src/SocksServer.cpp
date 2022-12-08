#include <sys/wait.h>

#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>


#ifndef _SOCKS_SERVER_H_
#define _SOCKS_SERVER_H_
#include "SocksServer.h"
#endif

#ifndef _SOCKS_SESSION_H_
#define _SOCKS_SESSION_H_
#include "SocksSession.h"
#endif

using namespace std;
using boost::asio::ip::tcp;


SocksServer::SocksServer(boost::asio::io_context& io_context, int port)
    : port(port), acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
    signal(SIGCHLD, SocksServer::childSignalHandler);
    doAccept(io_context);
}

void SocksServer::doAccept(boost::asio::io_context& io_context) {
    acceptor.async_accept([this, &io_context](boost::system::error_code errorCode, tcp::socket socket) {
        if (!errorCode) {
            make_shared<SocksSession>(std::move(socket), io_context)->start();
        }
        doAccept(io_context);
    });
}

void SocksServer::childSignalHandler(int signum) {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {
    }
}
