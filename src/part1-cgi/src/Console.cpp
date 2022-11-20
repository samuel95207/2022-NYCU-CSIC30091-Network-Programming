#include <algorithm>
#include <iostream>
#include <regex>

#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#include "Console.h"
#endif

#ifndef _CONSOLE_SESSION_H_
#define _CONSOLE_SESSION_H_
#include "ConsoleSession.h"
#endif


using namespace std;

map<string, string> Console::htmlEscapeMap = {
    {" ", "&nbsp;"},  {"\n", "&NewLine;"}, {">", "&gt;"},    {"<", "&lt;"},   {"\'", "&apos;"},
    {"\"", "&quot;"}, {"\\|", "&#124;"},   {"-", "&ndash;"}, {"\\\\", "\\\\"}

};


void Console::start() {
    try {
        getCgiEnv();
        // cout << "output<br/>";
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

            shared_ptr<ConsoleSession> ptr = std::make_shared<ConsoleSession>(io_context, this);
            sessions[id] = ptr;

            ptr->start(id, shellHost, shellPort, shellFilename, request);
        }

        io_context.run();


        while (true) {
            bool exit = true;
            for (auto sessionPair : sessions) {
                exit = exit && sessionPair.second->isExit();
            }
            if (exit) {
                break;
            }
        }

        renderHtml();

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


void Console::renderHtml() {
    cout << "HTTP/1.1 200 OK\r\n"
            "Content-type:text/html\r\n\r\n"
            "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "  <script>document.getElementById('body').innerHTML = ''</script>"
            "  <head>"
            "    <meta charset=\"UTF-8\" />"
            "    <title>NP Project 3 Sample Console</title>"
            "    <link"
            "      rel=\"stylesheet\""
            "      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\""
            "      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\""
            "      crossorigin=\"anonymous\""
            "    />"
            "    <link"
            "      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\""
            "      rel=\"stylesheet\""
            "    />"
            "    <link"
            "      rel=\"icon\""
            "      type=\"image/png\""
            "      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\""
            "    />"
            "    <style>"
            "      * {"
            "        font-family: 'Source Code Pro', monospace;"
            "        font-size: 1rem !important;"
            "      }"
            "      body {"
            "        background-color: #212529;"
            "      }"
            "      pre {"
            "        color: #cccccc;"
            "      }"
            "      b {"
            "        color: #01b468;"
            "      }"
            "    </style>"
            "  </head>"
            "  <body id='body'>"
            "    <table class=\"table table-dark table-bordered\">"
            "      <thead>"
            "        <tr>";
    for (auto sessionPair : sessions) {
        cout << "<th scope=\"col\">" << sessionPair.second->getHost() << ":" << sessionPair.second->getPort()
             << "</th>";
    }
    cout << "        </tr>"
            "      </thead>"
            "      <tbody>"
            "        <tr>";
    for (auto sessionPair : sessions) {
        cout << "<td><pre id=\"s" << sessionPair.first << "\" class=\"mb-0\">";
        for (auto commandResponse : sessionPair.second->getCommandResponseArr()) {
            if (commandResponse.type == CommandResponseType::COMMAND) {
                cout << renderCommand(commandResponse.value);
            } else {
                cout << renderResponse(commandResponse.value);
            }
        }
        cout << "</pre></td>";
    }
    cout << "        </tr>"
            "      </tbody>"
            "    </table>"
            "  </body>"
            "</html>";
    cout.flush();
}


string Console::renderCommand(string value) {
    value = regex_replace(value, std::regex("&"), "&amp;");
    for (auto escape : htmlEscapeMap) {
        value = regex_replace(value, std::regex(escape.first), escape.second);
    }
    return string("<b>") + value + string("</b>");
}

string Console::renderResponse(string value) {
    value = regex_replace(value, std::regex("&"), "&amp;");
    for (auto escape : htmlEscapeMap) {
        value = regex_replace(value, std::regex(escape.first), escape.second);
    }
    return value;
}