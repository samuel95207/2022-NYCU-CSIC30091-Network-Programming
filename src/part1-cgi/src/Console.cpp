#include <iostream>

#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#include "Console.h"
#endif

#ifndef _CONSOLE_SESSION_H_
#define _CONSOLE_SESSION_H_
#include "ConsoleSession.h"
#endif


using namespace std;


void Console::start() {
    try {

        getCgiEnv();

        cout << "HTTP/1.1 200 OK\r\n";
        cout << "Content-type:text/html\r\n\r\n";
        cout << "output<br/>";
        // request.print();

        for (auto queryPair : request.queryMap) {
            // cout << queryPair.first << " " << queryPair.second << "<br/>";
            if (queryPair.first[0] != 'h' || queryPair.second == "") {
                continue;
            }
            int id = stoi(queryPair.first.substr(1));
            string shellHost = queryPair.second;
            int shellPort = stoi(request.queryMap[string("p") + to_string(id)]);
            string shellFilename = request.queryMap[string("f") + to_string(id)];

            shared_ptr<ConsoleSession> ptr = std::make_shared<ConsoleSession>(io_context);
            sessions[id] = ptr;

            ptr->start(id, shellHost, shellPort, shellFilename, request);
        }

        io_context.run();
        // while (true) {
        //     for (auto sessionPair : sessions) {
        //         if (sessionPair.second->isExit()) {
        //             cout << "id=" << sessionPair.first << "<br/>";
        //             for (auto commandResponse : sessionPair.second->getCommandResponseArr()) {
        //                 cout << commandResponse.command << "|" << commandResponse.response << "<br/>";
        //             }
        //         }
        //     }
        // }
    } catch (std::exception& e) {
        cerr << "Exception: " << e.what() << "<br/>";
    }
}


void Console::getCgiEnv() {
    request.method = string(getenv("REQUEST_METHOD"));
    request.uri = string(getenv("REQUEST_URI"));
    request.queryStr = string(getenv("QUERY_STRING"));
    request.version = string(getenv("SERVER_PROTOCOL"));
    request.host = string(getenv("HTTP_HOST"));
    request.addr = string(getenv("SERVER_ADDR"));
    request.port = stoi(string(getenv("SERVER_PORT")));



    istringstream issQuery;
    issQuery.str(request.queryStr);
    string queryToken;
    while (getline(issQuery, queryToken, '&')) {
        if (queryToken.length() == 0) {
            continue;
        }
        istringstream queryTokenIss;
        queryTokenIss.str(queryToken);

        string key, value;
        getline(queryTokenIss, key, '=');
        getline(queryTokenIss, value);
        request.queryMap[key] = value;
    }
}