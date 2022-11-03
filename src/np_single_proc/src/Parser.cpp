#ifndef _PARSER_H_
#define _PARSER_H_
#include "Parser.h"
#endif

#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>

using namespace std;

const vector<string> Parser::operatorTypes = {"\\|",          "\\|[1-9][0-9]*", "\\![1-9][0-9]*",
                                              "<[1-9][0-9]*", ">[1-9][0-9]*",   ">"};
const vector<string> Parser::oneArgCommands = {"yell"};
const vector<string> Parser::twoArgCommands = {"tell"};



ParseResult Parser::parse(string commandStr) {
    ParseResult parseResult;
    istringstream iss(commandStr);

    string token;

    string command = "";
    vector<string> operators;
    vector<string> args;
    Command newCommand;


    while (iss >> token) {
        if (isOperator(token)) {
            operators.push_back(token);

            command = "";
        } else if (command == "") {
            if (newCommand.command != "") {
                newCommand.args = args;
                newCommand.operators = operators;
                parseResult.commands.push_back(newCommand);

                newCommand.command = "";
                args.clear();
                operators.clear();
            }

            command = token;
            newCommand.command = command;

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

    if (newCommand.command != "") {
        newCommand.args = args;
        newCommand.operators = operators;

        parseResult.commands.push_back(newCommand);

        args.clear();
        operators.clear();
    }


    if (parseResult.errorMessage != "") {
        parseResult.commands.clear();
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
        cout << "Command: " << command.command << endl;
        cout << "Args: ";
        for (auto arg : command.args) {
            cout << arg << " ";
        }
        cout << endl;
        cout << "Operators: ";
        for (auto op : command.operators) {
            cout << op << " ";
        }
        cout << endl << endl;
    }
}
