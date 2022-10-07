#include <iostream>

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif

#ifndef _PARSER_H_
#define _PARSER_H_
#include "Parser.h"
#endif

#ifndef _BUILDIN_COMMAND_H_
#define _BUILDIN_COMMAND_H_
#include "BuildinCommand.h"
#endif

using namespace std;

NPShell::NPShell() {}

void NPShell::run() {
    string command;
    while (cout << symbol && getline(cin, command)) {
        auto parseResult = Parser::parse(command);
        Parser::printParseResult(parseResult);


        executeCommand(parseResult.commands[0].first, parseResult.commands[0].second);

        if (exitFlag) {
            break;
        }
    }
}

void NPShell::setExit() { exitFlag = true; }


void NPShell::executeCommand(const std::string& command, const std::vector<std::string>& args) {
    if (BuildinCommand::isBuildinCommand(command)) {
        BuildinCommand::execute(*this, command, args);
    }
}