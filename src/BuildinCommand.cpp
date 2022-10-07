#ifndef _BUILDIN_COMMAND_H_
#define _BUILDIN_COMMAND_H_
#include "BuildinCommand.h"
#endif

#include <iostream>

using namespace std;


unordered_map<string, BuildinCommandFunction> BuildinCommand::commands = {{"exit", BuildinCommand::exit}};


bool BuildinCommand::isBuildinCommand(string command) { return commands.find(command) != commands.end(); }

bool BuildinCommand::execute(NPShell& shell, const string& command, const vector<string>& args){
    auto commandPairPtr = commands.find(command);
    if(commandPairPtr == commands.end()){
        return false;
    }
    auto commandFunc = commandPairPtr->second;
    return commandFunc(shell ,command, args);
}


bool BuildinCommand::exit(NPShell& shell, const string& command, const vector<string>& args) {
    shell.setExit();
    return true;
}
