#include <boost/asio.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <utility>



#ifndef _SOCKS_SESSION_H_
#define _SOCKS_SESSION_H_
#include "SocksSession.h"
#endif


#ifndef _SOCKS_REQUEST_H_
#define _SOCKS_REQUEST_H_
#include "Socks4aRequest.h"
#endif

using namespace std;
using boost::asio::ip::tcp;


SocksSession::SocksSession(tcp::socket socket, boost::asio::io_service& io_service)
    : clientSocket(std::move(socket)), serverSocket(io_service), acceptor(io_service), resolver(io_service) {
    firewall = Firewall("./socks.conf");
}

void SocksSession::start() { readSocks(); }


void SocksSession::readSocks() {
    auto self(shared_from_this());
    clientSocket.async_read_some(boost::asio::buffer(socksRequestBuf, BUF_SIZE),
                                 [this, self](boost::system::error_code errorCode, size_t length) {
                                     if (errorCode) {
                                         //  cerr << "readSocks(): " << errorCode.message() << endl;
                                         return;
                                     }
                                     request = Socks4aRequest::parse(socksRequestBuf);
                                     request.resolveDomainName(resolver);

                                     //    request.print();
                                     //    cout << endl;

                                     bool isPermit = firewall.isPermit(request);
                                     response = Socks4aResponse(isPermit);


                                     printSocksInfo();
                                     cout << endl;


                                     if (!response.isAccept()) {
                                         replySocks();
                                         return;
                                     }

                                     if (request.getCommandString() == "CONNECT") {
                                         createConnectTunnel();
                                     } else if (request.getCommandString() == "BIND") {
                                         createBindTunnel();
                                     } else {
                                         clientSocket.close();
                                     }
                                 });
}

void SocksSession::replySocks() {
    auto self(shared_from_this());

    response.setHton();
    // response.print();
    // cout << endl;

    boost::asio::async_write(clientSocket, boost::asio::buffer(&response, sizeof(response)),
                             [this, self](boost::system::error_code errorCode, std::size_t length) {
                                 if (errorCode) {
                                     //  cerr << "replySocks(): " << errorCode.message() << endl;
                                     return;
                                 }

                                 if (!response.isAccept()) {
                                     clientSocket.close();
                                 }
                             });
}

void SocksSession::createConnectTunnel() {
    auto self(shared_from_this());

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(request.getDstIpString()),
                                            request.dstPort);

    serverSocket.async_connect(endpoint, [this, self](const boost::system::error_code& errorCode) {
        if (errorCode) {
            response.setAccept(false);
            replySocks();

            clientSocket.close();

            // cerr << "createConnectTunnel(): " << errorCode.message() << endl;
            return;
        }

        replySocks();
        readFromClient();
        readFromServer();
    });
}


void SocksSession::createBindTunnel() {
    auto self(shared_from_this());

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4::any(), 0);
    acceptor.open(endpoint.protocol());
    acceptor.bind(endpoint);
    acceptor.listen();

    response.setDstPort(acceptor.local_endpoint().port());
    replySocks();

    acceptor.async_accept(serverSocket, [this, self](const boost::system::error_code& errorCode) {
        if (errorCode) {
            clientSocket.close();
            // cerr << "createBindTunnel(): " << errorCode.message() << endl;
            return;
        }

        replySocks();
        readFromClient();
        readFromServer();
    });
}

void SocksSession::readFromClient() {
    auto self(shared_from_this());

    memset(readFromClientBuf, 0, BUF_SIZE);
    clientSocket.async_read_some(boost::asio::buffer(readFromClientBuf, BUF_SIZE),
                                 [this, self](boost::system::error_code errorCode, std::size_t length) {
                                     if (errorCode) {
                                         clientSocket.close();
                                         serverSocket.close();
                                         //  cerr << "readFromClient(): " << errorCode.message() << endl;
                                         return;
                                     }

                                     memset(writeToServerBuf, 0, BUF_SIZE);
                                     memcpy(writeToServerBuf, readFromClientBuf, length);
                                     writeToServer(length);
                                 });
}

void SocksSession::readFromServer() {
    auto self(shared_from_this());

    memset(readFromServerBuf, 0, BUF_SIZE);
    serverSocket.async_read_some(boost::asio::buffer(readFromServerBuf, BUF_SIZE),
                                 [this, self](boost::system::error_code errorCode, std::size_t length) {
                                     if (errorCode) {
                                         clientSocket.close();
                                         serverSocket.close();
                                         //  cerr << "readFromServer(): " << errorCode.message() << endl;
                                         return;
                                     }

                                     memset(writeToClientBuf, 0, BUF_SIZE);
                                     memcpy(writeToClientBuf, readFromServerBuf, length);
                                     writeToClient(length);
                                 });
}


void SocksSession::writeToServer(std::size_t length) {
    auto self(shared_from_this());
    boost::asio::async_write(serverSocket, boost::asio::buffer(writeToServerBuf, length),
                             [this, self](boost::system::error_code errorCode, std::size_t length) {
                                 if (errorCode) {
                                     clientSocket.close();
                                     serverSocket.close();
                                     //  cerr << "writeToServer(): " << errorCode.message() << endl;
                                     return;
                                 }

                                 memset(writeToServerBuf, 0, BUF_SIZE);
                                 readFromClient();
                             });
}

void SocksSession::writeToClient(std::size_t length) {
    auto self(shared_from_this());
    boost::asio::async_write(clientSocket, boost::asio::buffer(writeToClientBuf, length),
                             [this, self](boost::system::error_code errorCode, std::size_t length) {
                                 if (errorCode) {
                                     clientSocket.close();
                                     serverSocket.close();
                                     //  cerr << "writeToClient(): " << errorCode.message() << endl;
                                     return;
                                 }
                                 memset(writeToClientBuf, 0, BUF_SIZE);
                                 readFromServer();
                             });
}

void SocksSession::printSocksInfo() {
    cout << "<S_IP>: " << clientSocket.remote_endpoint().address().to_string() << endl;
    cout << "<S_PORT>: " << to_string(clientSocket.remote_endpoint().port()) << endl;
    cout << "<D_IP>: " << request.getDstIpString() << endl;
    cout << "<D_PORT>: " << request.dstPort << endl;
    cout << "<Command>: " << request.getCommandString() << endl;
    cout << "<Reply>: " << response.getAcceptString() << endl;
}



// void SocksSession::doWrite() {
//     auto self(shared_from_this());
//     strcpy(data, "SOCKS/1.1 200 OK\r\n");
//     boost::asio::async_write(
//         socket, boost::asio::buffer(data, strlen(data)),
//         [this, self](boost::system::error_code errorCode, std::size_t length) {
//             if (errorCode) {
//                 cerr << "SOCKS/1.1 500 Internal Server Error\r\n";
//                 cerr << "Write Error! " << errorCode << endl;
//                 return;
//             }

//             pid_t pid;
//             do {
//                 pid = fork();
//             } while (pid < 0);

//             if (pid > 0) {
//                 // Parent Process
//                 socket.close();
//             } else {
//                 setCgiEnv();
//                 dup2(socket.native_handle(), fileno(stdin));
//                 dup2(socket.native_handle(), fileno(stdout));
//                 dup2(socket.native_handle(), fileno(stderr));
//                 socket.close();

//                 string cgiPath = "." + request.uriOnly;
//                 if (execlp(cgiPath.c_str(), cgiPath.c_str(), NULL) < 0) {
//                     cerr << "SOCKS/1.1 404 Not Found\r\n";
//                     cerr << "Content-type:text/html\r\n\r\n<h1>404 Not Found!</h1><h2>CGI does not exist.</h2>";
//                     exit(0);
//                 }
//             }
//         });
// }


// void SocksSession::recvRequest(string rawRequest) {
//     cout << rawRequest << endl;
//     request = Socks4aRequest::parse(rawRequest);

//     request.print();
//     cout << "REMOTE_ADDR = " << socket.remote_endpoint().address().to_string().c_str() << endl;
//     cout << "REMOTE_PORT = " << to_string(socket.remote_endpoint().port()).c_str() << endl;
// }

// void SocksSession::setCgiEnv() {
//     setenv("REQUEST_METHOD", request.method.c_str(), 1);
//     setenv("REQUEST_URI", request.uri.c_str(), 1);
//     setenv("QUERY_STRING", request.queryStr.c_str(), 1);
//     setenv("SERVER_PROTOCOL", request.version.c_str(), 1);
//     setenv("SOCKS_HOST", request.host.c_str(), 1);
//     setenv("SERVER_ADDR", request.addr.c_str(), 1);
//     setenv("SERVER_PORT", to_string(request.port).c_str(), 1);
//     setenv("REMOTE_ADDR", socket.remote_endpoint().address().to_string().c_str(), 1);
//     setenv("REMOTE_PORT", to_string(socket.remote_endpoint().port()).c_str(), 1);
// }
