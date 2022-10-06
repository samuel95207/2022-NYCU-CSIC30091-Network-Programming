#include <iostream>

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif

#ifndef _PARSER_H_
#define _PARSER_H_
#include "Parser.h"
#endif

using namespace std;

NPShell::NPShell() {}

void NPShell::run() {
    string command;
    while (cout << symbol && getline(cin, command)) {
        auto parseResult = Parser::parse(command);
        Parser::printParseResult(parseResult);
    }
}