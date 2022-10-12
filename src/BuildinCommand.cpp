#ifndef _BUILDIN_COMMAND_H_
#define _BUILDIN_COMMAND_H_
#include "BuildinCommand.h"
#endif

#include <cstdlib>
#include <iostream>

using namespace std;


unordered_map<string, BuildinCommandFunction> BuildinCommand::commands = {{"exit", BuildinCommand::exitCommand},
                                                                          {"printenv", BuildinCommand::printenvCommand},
                                                                          {"setenv", BuildinCommand::setenvCommand}};


bool BuildinCommand::isBuildinCommand(string command) { return commands.find(command) != commands.end(); }

bool BuildinCommand::execute(NPShell& shell, const string& command, const vector<string>& args) {
    auto commandPairPtr = commands.find(command);
    if (commandPairPtr == commands.end()) {
        return false;
    }
    auto commandFunc = commandPairPtr->second;
    return commandFunc(shell, command, args);
}


bool BuildinCommand::exitCommand(NPShell& shell, const string& command, const vector<string>& args) {
    shell.setExit();
    return true;
}

bool BuildinCommand::printenvCommand(NPShell& shell, const string& command, const vector<string>& args) {
    if (args.size() < 1) {
        return false;
    }

    auto envRaw = getenv(args[0].c_str());
    if(envRaw != nullptr){
        cout << string(envRaw) << endl;
        return true;
    }

    return true;
}

bool BuildinCommand::setenvCommand(NPShell& shell, const string& command, const vector<string>& args) {
    string value = "";
    if (args.size() < 1) {
        return false;
    } else if (args.size() >= 2) {
        value = args[1];
    }

    return !setenv(args[0].c_str(), value.c_str(), 1);
}