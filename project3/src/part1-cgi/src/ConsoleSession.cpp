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



ConsoleSession::ConsoleSession(boost::asio::io_service& io_service, Console* console, int id, string host, int port,
                               string filename, HttpRequest request)
    : socket(io_service),
      resolver(io_service),
      timer(io_service),
      console(console),
      id(id),
      host(host),
      port(port),
      filename(filename),
      request(request) {}

void ConsoleSession::start() {
    exit = false;

    scriptFile.open((scriptPath + filename).c_str(), ios::in);
    if (!scriptFile.is_open()) {
        addOutput(string("File Open Error! Cannot open file ") + scriptPath + filename + ".\n", OutputType::ERRORMSG);
        exitSession();
        return;
    }

    tcp::resolver::query query(host, to_string(port));

    resolver.async_resolve(query, boost::bind(&ConsoleSession::onResolve, shared_from_this(),
                                              boost::asio::placeholders::error, boost::asio::placeholders::iterator));
}

bool ConsoleSession::isExit() { return exit; }



string ConsoleSession::getHost() { return host; }
int ConsoleSession::getPort() { return port; }
vector<Output> ConsoleSession::getOutputArr() { return commandResponseArr; }


void ConsoleSession::onResolve(const boost::system::error_code& errorCode, tcp::resolver::iterator iterator) {
    if (errorCode) {
        addOutput(string("Resolve Error: ") + errorCode.message() + "\n", OutputType::ERRORMSG);
        exitSession();
        return;
    }

    tcp::endpoint endpoint = *iterator;
    socket.async_connect(endpoint, boost::bind(&ConsoleSession::onConnect, shared_from_this(),
                                               boost::asio::placeholders::error, ++iterator));
}

void ConsoleSession::onConnect(const boost::system::error_code& errorCode, tcp::resolver::iterator iterator) {
    if (errorCode) {
        addOutput(string("Connect Error: ") + errorCode.message() + "\n", OutputType::ERRORMSG);
        exitSession();
        return;
    }


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
                                   addOutput(string("Read Error: ") + errorCode.message() + "\n", OutputType::ERRORMSG);
                                   exitSession();
                                   return;
                               }

                               string rawRequest = string(data);

                               //    cout << id << " read: " << rawRequest << "<br/>";


                               addOutput(rawRequest, OutputType::RESPONSE);

                               if (rawRequest.find("% ") != string::npos) {
                                   doWrite();
                               } else {
                                   doRead();
                               }
                           });
}

void ConsoleSession::doWrite() {
    auto self(shared_from_this());
    strcpy(data, "");
    memset(data, 0, BUF_SIZE);



    timer.expires_from_now(boost::posix_time::seconds(3));
    timer.async_wait([this, self](boost::system::error_code errorCode) {
        if (errorCode) {
            addOutput(string("Timer Error: ") + errorCode.message() + "\n", OutputType::ERRORMSG);
            exitSession();
            return;
        }
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
                                         addOutput(string("Write Error: ") + errorCode.message() + "\n",
                                                   OutputType::ERRORMSG);
                                         exitSession();
                                         return;
                                     }

                                     addOutput(command, OutputType::COMMAND);

                                     if (command == "exit\r\n" || command == "exit\n") {
                                         socket.close();
                                         scriptFile.close();

                                         exitSession();
                                         return;

                                     } else {
                                         doRead();
                                     }
                                 });
    });
}


void ConsoleSession::addOutput(string output, OutputType type) {
    Output newOutput;
    newOutput.value = output;
    newOutput.type = type;
    commandResponseArr.push_back(newOutput);
    console->renderOutput(id, newOutput);
}

void ConsoleSession::exitSession() {
    exit = true;
    socket.close();
}