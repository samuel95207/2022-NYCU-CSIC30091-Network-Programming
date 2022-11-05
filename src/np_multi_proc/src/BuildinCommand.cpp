#ifndef _BUILDIN_COMMAND_H_
#define _BUILDIN_COMMAND_H_
#include "BuildinCommand.h"
#endif

#ifndef _MULTI_PROC_SERVER_H_
#define _MULTI_PROC_SERVER_H_
#include "MultiProcServer.h"
#endif

#include <cstdlib>
#include <iostream>

using namespace std;


unordered_map<string, BuildinCommandFunction> BuildinCommand::commands = {
    {"exit", BuildinCommand::exitCommand},     {"printenv", BuildinCommand::printenvCommand},
    {"setenv", BuildinCommand::setenvCommand}, {"who", BuildinCommand::whoCommand},
    {"yell", BuildinCommand::yellCommand},     {"name", BuildinCommand::nameCommand},
    {"tell", BuildinCommand::tellCommand}};


bool BuildinCommand::isBuildinCommand(string command) { return commands.find(command) != commands.end(); }

bool BuildinCommand::execute(NPShell& shell, MultiProcServer& server, int pid, int fd, const string& command,
                             const vector<string>& args) {
    auto commandPairPtr = commands.find(command);
    if (commandPairPtr == commands.end()) {
        return false;
    }
    auto commandFunc = commandPairPtr->second;
    return commandFunc(shell, server, pid, fd, command, args);
}


bool BuildinCommand::exitCommand(NPShell& shell, MultiProcServer& server, int pid, int fd, const string& command,
                                 const vector<string>& args) {
    shell.setExit();
    return true;
}

bool BuildinCommand::printenvCommand(NPShell& shell, MultiProcServer& server, int pid, int fd, const string& command,
                                     const vector<string>& args) {
    if (args.size() < 1) {
        return false;
    }

    cout << string(shell.getEnv(args[0])) << endl;

    return true;
}

bool BuildinCommand::setenvCommand(NPShell& shell, MultiProcServer& server, int pid, int fd, const string& command,
                                   const vector<string>& args) {
    string value = "";
    if (args.size() < 1) {
        return false;
    } else if (args.size() >= 2) {
        value = args[1];
    }

    shell.setEnv(args[0], value);
    return true;
}


bool BuildinCommand::whoCommand(NPShell& shell, MultiProcServer& server, int pid, int fd, const string& command,
                                const vector<string>& args) {
    cout << "<ID>\t<nickname>\t<IP:port>\t<indicate me>" << endl;
    User* me = server.userManager.getUserByPid(pid);
    for (auto idUserPair : server.userManager.getIdUserMap()) {
        User* user = idUserPair.second;
        cout << user->id << "\t" << (user->name == "" ? "(no name)" : user->name) << "\t" << user->ipAddr
             << (user->id == me->id ? "\t<-me" : "") << endl;
    }

    return true;
}


bool BuildinCommand::yellCommand(NPShell& shell, MultiProcServer& server, int pid, int fd, const string& command,
                                 const vector<string>& args) {
    User* me = server.userManager.getUserByPid(pid);
    string message = "*** " + (me->name == "" ? "(no name)" : me->name) + " yelled ***: " + args[0] + "\n";
    server.broadcast(message);

    return true;
}


bool BuildinCommand::nameCommand(NPShell& shell, MultiProcServer& server, int pid, int fd, const string& command,
                                 const vector<string>& args) {
    User* me = server.userManager.getUserByPid(pid);
    string name = args[0];

    if (name == "") {
        cerr << "Error args1 cannot be empty" << endl;
        return false;
    }

    cout << me->id << endl;
    if (!server.userManager.setNameById(me->id, name)) {
        cerr << "*** User '" << name << "' already exists. ***" << endl;
        return false;
    }

    me->name = name;
    string message = "*** User from " + me->ipAddr + " is named '" + me->name + "'. ***\n";
    server.broadcast(message);

    return true;
}


bool BuildinCommand::tellCommand(NPShell& shell, MultiProcServer& server, int pid, int fd, const string& command,
                                 const vector<string>& args) {
    User* me = server.userManager.getUserByPid(pid);

    int toId = stoi(args[0]);
    string rawMessage = args[1];

    User* toUser = server.userManager.getUserById(toId);
    if (toUser == nullptr) {
        cerr << "*** Error: user #" << toId << " does not exist yet. ***" << endl;
        return false;
    }

    string message = "*** " + (me->name == "" ? "(no name)" : me->name) + " told you ***: " + rawMessage + "\n";
    server.sendDirectMessage(toUser->id, message);

    return true;
}