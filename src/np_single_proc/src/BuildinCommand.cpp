#ifndef _BUILDIN_COMMAND_H_
#define _BUILDIN_COMMAND_H_
#include "BuildinCommand.h"
#endif

#ifndef _SINGLE_PROC_SERVER_H_
#define _SINGLE_PROC_SERVER_H_
#include "SingleProcServer.h"
#endif

#include <cstdlib>
#include <iostream>

using namespace std;


unordered_map<string, BuildinCommandFunction> BuildinCommand::commands = {
    {"exit", BuildinCommand::exitCommand},     {"printenv", BuildinCommand::printenvCommand},
    {"setenv", BuildinCommand::setenvCommand}, {"who", BuildinCommand::whoCommand},
    {"yell", BuildinCommand::yellCommand},     {"name", BuildinCommand::nameCommand}};


bool BuildinCommand::isBuildinCommand(string command) { return commands.find(command) != commands.end(); }

bool BuildinCommand::execute(NPShell& shell, SingleProcServer& server, int fd, const string& command,
                             const vector<string>& args) {
    auto commandPairPtr = commands.find(command);
    if (commandPairPtr == commands.end()) {
        return false;
    }
    auto commandFunc = commandPairPtr->second;
    return commandFunc(shell, server, fd, command, args);
}


bool BuildinCommand::exitCommand(NPShell& shell, SingleProcServer& server, int fd, const string& command,
                                 const vector<string>& args) {
    shell.setExit();
    return true;
}

bool BuildinCommand::printenvCommand(NPShell& shell, SingleProcServer& server, int fd, const string& command,
                                     const vector<string>& args) {
    if (args.size() < 1) {
        return false;
    }

    auto envRaw = getenv(args[0].c_str());
    if (envRaw == nullptr) {
        return true;
    }

    cout << string(envRaw) << endl;

    return true;
}

bool BuildinCommand::setenvCommand(NPShell& shell, SingleProcServer& server, int fd, const string& command,
                                   const vector<string>& args) {
    string value = "";
    if (args.size() < 1) {
        return false;
    } else if (args.size() >= 2) {
        value = args[1];
    }

    return !setenv(args[0].c_str(), value.c_str(), 1);
}


bool BuildinCommand::whoCommand(NPShell& shell, SingleProcServer& server, int fd, const string& command,
                                const vector<string>& args) {
    cout << "<ID>\t<nickname>\t<IP:port>\t<indicate me>" << endl;

    User* me = server.userManager.getUserByFd(fd);
    for (auto idUserPair : server.userManager.getIdUserMap()) {
        User* user = idUserPair.second;
        string ipString = string(inet_ntoa(user->ipAddr.sin_addr)) + ":" + to_string((int)ntohs(user->ipAddr.sin_port));

        cout << user->id << "\t" << (user->name == "" ? "(no name)" : user->name) << "\t" << ipString << "\t"
             << (user == me ? "<-me" : "") << endl;
    }

    return true;
}


bool BuildinCommand::yellCommand(NPShell& shell, SingleProcServer& server, int fd, const string& command,
                                 const vector<string>& args) {
    User* me = server.userManager.getUserByFd(fd);
    string message = "*** " + (me->name == "" ? "(no name)" : me->name) + " yelled ***: " + args[0] + "\n";
    server.broadcast(message);

    return true;
}


bool BuildinCommand::nameCommand(NPShell& shell, SingleProcServer& server, int fd, const string& command,
                                 const vector<string>& args) {
    User* me = server.userManager.getUserByFd(fd);
    string name = args[0];

    if (name == "") {
        cerr << "Error args1 cannot be empty" << endl;
        return false;
    }
    if (!server.userManager.setNameById(me->id, name)) {
        cerr << "*** User '" << name << "' already exists. ***" << endl;
        return false;
    }

    me->name = name;
    string ipString = string(inet_ntoa(me->ipAddr.sin_addr)) + ":" + to_string((int)ntohs(me->ipAddr.sin_port));
    string message = "*** User from " + ipString + " is named '" + me->name + "'. ***\n";
    server.broadcast(message);

    return true;
}