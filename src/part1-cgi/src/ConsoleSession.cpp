#include <boost/asio.hpp>
#include <boost/bind.hpp>
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

#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#include "Console.h"
#endif


#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_
#include "HttpRequest.h"
#endif

using namespace std;
using boost::asio::ip::tcp;

const string ConsoleSession::scriptPath = "./test_case/";



ConsoleSession::ConsoleSession(boost::asio::io_service& io_service, Console* console)
    : socket(io_service), resolver(io_service), console(console) {}

void ConsoleSession::start(int idIn, string hostIn, int portIn, string filenameIn, HttpRequest requestIn) {
    id = idIn;
    host = hostIn;
    port = portIn;
    filename = filenameIn;
    request = requestIn;
    exit = false;


    // cout << "id = " << id << " start!"


    scriptFile.open((scriptPath + filename).c_str(), ios::in);

    // cout << "filepath = " << scriptPath + filename << " " << scriptFile.is_open() << "<br/>";

    tcp::resolver::query query(host, to_string(port));

    // cout << "id " << id << " query<br/>";

    resolver.async_resolve(query, boost::bind(&ConsoleSession::onResolve, shared_from_this(),
                                              boost::asio::placeholders::error, boost::asio::placeholders::iterator));
}

bool ConsoleSession::isExit() { return exit; }



string ConsoleSession::getHost() { return host; }
int ConsoleSession::getPort() { return port; }
vector<CommandResponse> ConsoleSession::getCommandResponseArr() { return commandResponseArr; }


void ConsoleSession::onResolve(const boost::system::error_code& errorCode, tcp::resolver::iterator iterator) {
    if (errorCode) {
        std::cout << "Error: " << errorCode.message() << "<br/>";
    }

    // cout << "id " << id << " resolved<br/>";

    tcp::endpoint endpoint = *iterator;
    socket.async_connect(endpoint, boost::bind(&ConsoleSession::onConnect, shared_from_this(),
                                               boost::asio::placeholders::error, ++iterator));
}

void ConsoleSession::onConnect(const boost::system::error_code& errorCode, tcp::resolver::iterator iterator) {
    if (errorCode) {
        std::cout << "Error: " << errorCode.message() << "<br/>";
    }

    // cout << "id " << id << " connected<br/>";

    if (iterator != tcp::resolver::iterator()) {
        socket.close();
        tcp::endpoint endpoint = *iterator;
        socket.async_connect(endpoint, boost::bind(&ConsoleSession::onConnect, shared_from_this(),
                                                   boost::asio::placeholders::error, ++iterator));
    }

    doRead();
}


void ConsoleSession::doRead() {
    auto self(shared_from_this());
    strcpy(data, "");
    memset(data, 0, BUF_SIZE);

    socket.async_read_some(boost::asio::buffer(data, BUF_SIZE),
                           [this, self](boost::system::error_code errorCode, size_t length) {
                               if (errorCode) {
                                   std::cout << "Read Error: " << errorCode.message() << "<br/>";
                                   socket.close();
                                   return;
                               }

                               string rawRequest = string(data);

                               //    cout << id << " read: " << rawRequest << "<br/>";

                               CommandResponse newCommandResponse;
                               newCommandResponse.value = rawRequest;
                               newCommandResponse.type = CommandResponseType::RESPONSE;
                               commandResponseArr.push_back(newCommandResponse);

                               if (rawRequest.find("% ") != string::npos) {
                                   doWrite();
                               } else {
                                   console->renderHtml();
                                   doRead();
                               }
                           });
}

void ConsoleSession::doWrite() {
    auto self(shared_from_this());
    strcpy(data, "");
    memset(data, 0, BUF_SIZE);

    string command;
    getline(scriptFile, command);
    if (command[command.length() - 1] == '\r') {
        command.erase(command.length() - 1);
    }
    command += "\n";
    strcpy(data, command.c_str());

    boost::asio::async_write(socket, boost::asio::buffer(data, strlen(data)),
                             [this, self, command](boost::system::error_code errorCode, std::size_t length) {
                                 if (errorCode) {
                                     cerr << "Write Error! " << errorCode << endl;
                                     return;
                                 }
                                 // cout << id << " write: " << command << "<br/>";

                                 CommandResponse newCommandResponse;
                                 newCommandResponse.value = command;
                                 newCommandResponse.type = CommandResponseType::COMMAND;
                                 commandResponseArr.push_back(newCommandResponse);

                                 if (command == "exit\r\n" || command == "exit\n") {
                                     // cout << "id=" << id << " exiting <br/>";

                                    //  socket.close();
                                     scriptFile.close();


                                     // cout << "<br/>id=" << id << " Command and Response<br/>";
                                     // for (auto commandResponse : getCommandResponseArr()) {
                                     //     cout << (commandResponse.type == CommandResponseType::COMMAND ?
                                     //     "COMMAND" : "RESPONSE") << "|"
                                     //          << commandResponse.value << "|<br/>";
                                     // }


                                     exit = true;
                                 } else {
                                     doRead();
                                 }
                             });
}


void ConsoleSession::recvRequest(string rawRequest) {
    cout << rawRequest << endl;
    request = HttpRequest::parse(rawRequest);

    request.print();
    cout << "REMOTE_ADDR = " << socket.remote_endpoint().address().to_string().c_str() << endl;
    cout << "REMOTE_PORT = " << to_string(socket.remote_endpoint().port()).c_str() << endl;
}
