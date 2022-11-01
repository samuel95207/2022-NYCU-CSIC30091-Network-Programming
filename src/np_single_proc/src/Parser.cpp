#ifndef _PARSER_H_
#define _PARSER_H_
#include "Parser.h"
#endif

#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>

using namespace std;

const vector<string> Parser::operatorTypes = {"\\|", "\\|[1-9][0-9]*", "\\![1-9][0-9]*", ">"};
const vector<string> Parser::oneArgCommands = {"yell"};
const vector<string> Parser::twoArgCommands = {"tell"};



ParseResult Parser::parse(string commandStr) {
    ParseResult parseResult;
    istringstream iss(commandStr);

    string token;

    string command = "";
    vector<string> args;

    while (iss >> token) {
        if (isOperator(token)) {
            if (command == "") {
                parseResult.errorMessage = "Error! operator before command.";
                break;
            }
            parseResult.operators.push_back(token);

            parseResult.commands.push_back(pair<string, vector<string>>(command, args));
            command = "";
            args.clear();
        } else if (command == "") {
            command = token;

            if (isOneArgCommand(command)) {
                string arg;
                iss >> std::ws;
                getline(iss, arg);
                args.push_back(arg);
            } else if (isTwoArgCommand(command)) {
                string arg1, arg2;
                iss >> arg1;
                iss >> std::ws;
                getline(iss, arg2);
                args.push_back(arg1);
                args.push_back(arg2);
            }
        } else {
            args.push_back(token);
        }
    }

    if (command != "") {
        parseResult.commands.push_back(pair<string, vector<string>>(command, args));
    }


    if (parseResult.errorMessage != "") {
        parseResult.commands.clear();
        parseResult.operators.clear();
    }
    return parseResult;
}

bool Parser::isOperator(std::string token) {
    for (auto operatorType : operatorTypes) {
        if (regex_match(token, regex(operatorType))) {
            return true;
        }
    }
    return false;
}

bool Parser::isOneArgCommand(std::string command) {
    for (auto oneAgrCommand : oneArgCommands) {
        if (regex_match(command, regex(oneAgrCommand))) {
            return true;
        }
    }
    return false;
}

bool Parser::isTwoArgCommand(std::string command) {
    for (auto twoAgrCommand : twoArgCommands) {
        if (regex_match(command, regex(twoAgrCommand))) {
            return true;
        }
    }
    return false;
}

void Parser::printParseResult(const ParseResult& parseResult) {
    if (parseResult.errorMessage != "") {
        cout << parseResult.errorMessage << endl;
        return;
    }
    if (parseResult.commands.size() == 0) {
        return;
    }
    for (int i = 0; i < int(parseResult.commands.size()); i++) {
        auto command = parseResult.commands[i];
        cout << "Command: " << command.first << endl;
        cout << "Args: ";
        for (auto arg : command.second) {
            cout << arg << " ";
        }
        cout << endl << endl;

        if (i < int(parseResult.operators.size())) {
            cout << "Operator: " << parseResult.operators[i] << endl << endl;
        }
    }
}
