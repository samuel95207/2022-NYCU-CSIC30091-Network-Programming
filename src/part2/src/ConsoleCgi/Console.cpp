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
using boost::asio::ip::tcp;


map<string, string> Console::htmlEscapeMap = {
    {" ", "&nbsp;"},  {"\n", "&NewLine;"}, {">", "&gt;"},    {"<", "&lt;"},   {"\'", "&apos;"},
    {"\"", "&quot;"}, {"\\|", "&#124;"},   {"-", "&ndash;"}, {"\\\\", "\\\\"}

};

Console::Console(tcp::socket socket) : socket(std::move(socket)) {}

void Console::start(const HttpRequest& requestIn) {
    try {
        request = requestIn;
        writeSocket("Content-type:text/html\r\n\r\n");
        // request.print();

        for (auto queryPair : request.queryMap) {
            if (queryPair.first[0] != 'h' || queryPair.second == "") {
                continue;
            }
            int id = stoi(queryPair.first.substr(1));
            string shellHost = queryPair.second;
            int shellPort = stoi(request.queryMap[string("p") + to_string(id)]);
            string shellFilename = request.queryMap[string("f") + to_string(id)];

            shared_ptr<ConsoleSession> ptr =
                std::make_shared<ConsoleSession>(io_context, this, id, shellHost, shellPort, shellFilename, request);
            sessions[id] = ptr;
        }

        renderHtml();

        for (auto session : sessions) {
            session.second->start();
        }


        io_context.run();




    } catch (std::exception& e) {
        cerr << "Exception: " << e.what() << "<br/>";
    }
}


void Console::writeSocket(string message) {
    auto self(shared_from_this());
    std::strcpy(data, message.c_str());
    boost::asio::async_write(socket, boost::asio::buffer(data, strlen(data)),
                             [this, self](boost::system::error_code errorCode, std::size_t length) {
                                 if (errorCode) {
                                     cerr << "Write Error! " << errorCode << endl;
                                     return;
                                 }
                             });
}


void Console::renderHtml() {
    ostringstream oss;
    oss << "<!DOCTYPE html>"
           "<html lang=\"en\">"
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
           "      .error {"
           "        color: #f14c4c;"
           "      }"
           "    </style>"
           "  </head>"
           "  <body id='body'>"
           "    <table class=\"table table-dark table-bordered\">"
           "      <thead>"
           "        <tr>";
    for (auto sessionPair : sessions) {
        oss << "<th scope=\"col\">" << sessionPair.second->getHost() << ":" << sessionPair.second->getPort() << "</th>";
    }
    oss << "        </tr>"
           "      </thead>"
           "      <tbody>"
           "        <tr>";
    for (auto sessionPair : sessions) {
        oss << "<td><pre id=\"s" << sessionPair.first << "\" class=\"mb-0\"></pre></td>";
    }
    oss << "        </tr>"
           "      </tbody>"
           "    </table>"
           "  </body>"
           "</html>";

    writeSocket(oss.str());
}


void Console::renderOutput(int sessionId, Output& output) {
    string value = regex_replace(output.value, std::regex("&"), "&amp;");
    for (auto escape : htmlEscapeMap) {
        value = regex_replace(value, std::regex(escape.first), escape.second);
    }

    if (output.type == OutputType::COMMAND) {
        value = string("<b>") + value + string("</b>");
    } else if (output.type == OutputType::ERRORMSG) {
        value = string("<b class=\"error\">") + value + string("</b>");
    }

    ostringstream oss;
    oss << "<script>document.getElementById('s" << sessionId << "').innerHTML += '" << value << "';</script>";
    writeSocket(oss.str());
}