#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

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

#ifndef _UTIL_H_
#define _UTIL_H_
#include "Util.h"
#endif

#ifndef _MULTI_PROC_SERVER_H_
#define _MULTI_PROC_SERVER_H_
#include "MultiProcServer.h"
#endif

using namespace std;

NPShell::NPShell() {
    historyFilePath = string(getpwuid(getuid())->pw_dir) + "/.npshell_history";
    // cout << historyFilePath << endl;
}

void NPShell::execute(string commandRaw, MultiProcServer &server, int pid, int fd) {
    loadEnv();
    auto parseResult = Parser::parse(commandRaw);
    // Parser::printParseResult(parseResult);

    ofstream outfile(historyFilePath.c_str(), ios::app);
    outfile << commandRaw << endl;
    outfile.close();

    for (int commandIdx = 0; commandIdx < int(parseResult.commands.size()); commandIdx++) {
        auto command = parseResult.commands[commandIdx].command;
        auto args = parseResult.commands[commandIdx].args;
        string firstOperator = (parseResult.commands[commandIdx].operators.size() > 0)
                                   ? parseResult.commands[commandIdx].operators[0]
                                   : "";
        string secondOperator = (parseResult.commands[commandIdx].operators.size() > 1)
                                    ? parseResult.commands[commandIdx].operators[1]
                                    : "";
        string thirdOperator = (parseResult.commands[commandIdx].operators.size() > 2)
                                   ? parseResult.commands[commandIdx].operators[2]
                                   : "";


        // cout << "execute " << command << endl;


        bool doubleUserPipeFlag = false;
        int toUserId;
        int fromUserId;

        // Filter buildin commands
        if (BuildinCommand::isBuildinCommand(command)) {
            BuildinCommand::execute(*this, server, pid, fd, command, args);

            pipeManager.newSession();
            continue;
        }

        if (firstOperator == "|") {
            // To next normal pipe
            executeForkedCommand(command, args, PipeMode::NORMAL_PIPE);
        } else if (firstOperator == ">") {
            // To file
            if (commandIdx + 1 >= int(parseResult.commands.size())) {
                cerr << "Error! Filename cannot be empty." << endl;
                break;
            }
            string filename = parseResult.commands[commandIdx + 1].command;
            executeForkedCommand(command, args, PipeMode::FILE_OUTPUT, PipeMode::NONE, filename);

            pipeManager.newSession();
            break;
        } else if (firstOperator[0] == '|') {
            // To numbered pipe
            int count;
            sscanf(firstOperator.c_str(), "|%d", &count);

            pipeManager.addNumberedPipe(count);
            executeForkedCommand(command, args, PipeMode::NUMBERED_PIPE);

            pipeManager.newSession();
        } else if (firstOperator[0] == '!') {
            // To STDERR numbered pipe
            int count;
            sscanf(firstOperator.c_str(), "!%d", &count);

            pipeManager.addNumberedPipe(count);
            executeForkedCommand(command, args, PipeMode::NUMBERED_PIPE_STDERR);

            pipeManager.newSession();
        } else if (firstOperator[0] == '>') {
            sscanf(firstOperator.c_str(), ">%d", &toUserId);
            if (secondOperator != "<" && secondOperator[0] == '<') {
                doubleUserPipeFlag = true;
                sscanf(secondOperator.c_str(), "<%d", &fromUserId);

            } else {
                if (server.userManager.getUserById(toUserId).pid == -1) {
                    cerr << "*** Error: user #" << toUserId << " does not exist yet. ***" << endl;
                } else {
                    User me = server.userManager.getUserByPid(pid);
                    User toUser = server.userManager.getUserById(toUserId);
                    if (!pipeManager.addUserPipe(me.id, toUserId)) {
                        cerr << "*** Error: the pipe #" << me.id << "->#" << toUserId << " already exists. ***" << endl;
                    } else {
                        string message = "*** " + (me.name == "" ? "(no name)" : me.name) + " (#" + to_string(me.id) +
                                         ") just piped '" + commandRaw + "' to " +
                                         (toUser.name == "" ? "(no name)" : toUser.name) + " (#" + to_string(toUserId) +
                                         ") ***";
                        server.broadcast(message);

                        Message icpMessage;
                        icpMessage.pid = me.pid;
                        icpMessage.type = "openFromUserPipe";
                        icpMessage.value = to_string(me.id) + "_" + to_string(toUser.id);
                        MessageManager::addMessage(icpMessage);
                    }
                }

                executeForkedCommand(command, args, PipeMode::USER_PIPE_OUT);

                pipeManager.newSession();

                break;
            }
        } else if (firstOperator[0] == '<') {
            sscanf(firstOperator.c_str(), "<%d", &fromUserId);
            if (secondOperator != ">" && secondOperator[0] == '>') {
                doubleUserPipeFlag = true;
                sscanf(secondOperator.c_str(), ">%d", &toUserId);
            } else {
                if (server.userManager.getUserById(fromUserId).pid == -1) {
                    cerr << "*** Error: user #" << fromUserId << " does not exist yet. ***" << endl;
                } else {
                    User fromUser = server.userManager.getUserById(fromUserId);
                    User me = server.userManager.getUserByPid(pid);

                    if (!pipeManager.loadUserPipe(fromUserId, me.id)) {
                        cerr << "*** Error: the pipe #" << fromUserId << "->#" << me.id << " does not exist yet. ***"
                             << endl;
                    } else {
                        string message = "*** " + (me.name == "" ? "(no name)" : me.name) + " (#" + to_string(me.id) +
                                         ") just received from " + (fromUser.name == "" ? "(no name)" : fromUser.name) +
                                         " (#" + to_string(fromUserId) + ") by '" + commandRaw + "' ***";
                        server.broadcast(message);

                        Message icpMessage;
                        icpMessage.pid = fromUser.pid;
                        icpMessage.type = "closeFromUserPipe";
                        icpMessage.value = to_string(fromUser.id) + "_" + to_string(me.id);
                        MessageManager::addMessage(icpMessage);
                    }
                }

                PipeMode pipeMode2 = PipeMode::CONSOLE_OUTPUT;
                string filename = "";
                if (secondOperator == "|") {
                    pipeMode2 = PipeMode::NORMAL_PIPE;
                } else if (secondOperator == ">") {
                    pipeMode2 = PipeMode::FILE_OUTPUT;
                    if (commandIdx + 1 >= int(parseResult.commands.size())) {
                        cerr << "Error! Filename cannot be empty." << endl;
                        break;
                    }
                    filename = parseResult.commands[commandIdx + 1].command;
                } else if (secondOperator[0] == '|') {
                    pipeMode2 = PipeMode::NUMBERED_PIPE;
                    int count;
                    sscanf(secondOperator.c_str(), "|%d", &count);
                    pipeManager.addNumberedPipe(count);
                } else if (secondOperator[0] == '!') {
                    pipeMode2 = PipeMode::NUMBERED_PIPE_STDERR;
                    int count;
                    sscanf(secondOperator.c_str(), "!%d", &count);
                    pipeManager.addNumberedPipe(count);
                }




                executeForkedCommand(command, args, PipeMode::USER_PIPE_IN, pipeMode2, filename);

                if (pipeMode2 != PipeMode::NORMAL_PIPE) {
                    pipeManager.newSession();
                }
            }
        } else {
            // To console
            executeForkedCommand(command, args, PipeMode::CONSOLE_OUTPUT);


            pipeManager.newSession();
        }

        if (doubleUserPipeFlag) {
            User me = server.userManager.getUserByPid(pid);
            User fromUser = server.userManager.getUserById(fromUserId);
            User toUser = server.userManager.getUserById(toUserId);

            // cout << fromUserId << " " << toUserId << endl;

            if (server.userManager.getUserById(fromUserId).pid == -1) {
                cerr << "*** Error: user #" << fromUserId << " does not exist yet. ***" << endl;
            } else {
                if (!pipeManager.loadUserPipe(fromUserId, me.id)) {
                    cerr << "*** Error: the pipe #" << fromUserId << "->#" << me.id << " does not exist yet. ***"
                         << endl;
                } else {
                    string message = "*** " + (me.name == "" ? "(no name)" : me.name) + " (#" + to_string(me.id) +
                                     ") just received from " + (fromUser.name == "" ? "(no name)" : fromUser.name) +
                                     " (#" + to_string(fromUserId) + ") by '" + commandRaw + "' ***";
                    server.broadcast(message);
                }
            }

            if (server.userManager.getUserById(toUserId).pid == -1) {
                cerr << "*** Error: user #" << toUserId << " does not exist yet. ***" << endl;
            } else {
                if (!pipeManager.addUserPipe(me.id, toUserId)) {
                    cerr << "*** Error: the pipe #" << me.id << "->#" << toUserId << " already exists. ***" << endl;
                } else {
                    string message = "*** " + (me.name == "" ? "(no name)" : me.name) + " (#" + to_string(me.id) +
                                     ") just piped '" + commandRaw + "' to " +
                                     (toUser.name == "" ? "(no name)" : toUser.name) + " (#" + to_string(toUserId) +
                                     ") ***";
                    server.broadcast(message);
                }
            }

            Message icpMessage;
            icpMessage.pid = server.userManager.getUserById(fromUserId).pid;
            icpMessage.type = "closeFromUserPipe";
            icpMessage.value = to_string(fromUserId) + "_" + to_string(me.id);
            MessageManager::addMessage(icpMessage);

            icpMessage.pid = me.pid;
            icpMessage.type = "openFromUserPipe";
            icpMessage.value = to_string(me.id) + "_" + to_string(toUserId);
            MessageManager::addMessage(icpMessage);

            executeForkedCommand(command, args, PipeMode::USER_PIPE_BOTH);
            pipeManager.newSession();

            break;
        }
    }



    resetEnv();
}

string NPShell::getSymbol() { return symbol; }

bool NPShell::executeForkedCommand(const std::string &command, const std::vector<std::string> &args, PipeMode pipeMode,
                                   PipeMode pipeMode2, std::string outFilename) {
    if (!pipeManager.rootPipeHandler(pipeMode, pipeMode2, outFilename)) {
        cerr << "Pipe error!" << endl;
        return false;
    }

    pid_t pid = fork();

    if (pid == -1) {
        if (pipeMode == PipeMode::NORMAL_PIPE || pipeMode == PipeMode::NUMBERED_PIPE ||
            pipeMode == PipeMode::NUMBERED_PIPE_STDERR) {
            waitpid(-1, NULL, 0);
        } else {
            cerr << "Fork error!" << endl;
            return false;
        }
    } else if (pid > 0) {
        // Parent Process
        if (!pipeManager.parentPipeHandler(pipeMode, pipeMode2, outFilename)) {
            cerr << "Pipe error!" << endl;
            return false;
        }

        if (pipeMode == PipeMode::CONSOLE_OUTPUT || pipeMode == PipeMode::FILE_OUTPUT ||
            pipeMode2 == PipeMode::CONSOLE_OUTPUT || pipeMode2 == PipeMode::FILE_OUTPUT) {
            int status;

            waitpid(pid, &status, 0);
        }
    } else {
        // Children Process
        if (!pipeManager.childPipeHandler(pipeMode, pipeMode2, outFilename)) {
            cerr << "Pipe error!" << endl;
            return false;
        }

        execvp(command.c_str(), (char **)Util::createArgv(command, args));

        // Catch file not found error
        // cerr << "errno = " << errno << endl;
        if (errno == 13 || errno == 2) {
            cerr << "Unknown command: [" << command << "]." << endl;
        }
        exit(0);
    }

    return true;
}


void NPShell::setEnv(std::string env, std::string value) {
    auto envRaw = getenv(env.c_str());
    if (envMap.find(env) == envMap.end()) {
        if (envRaw != nullptr) {
            envMap[env] = pair<string, string>(string(envRaw), value);
        } else {
            envMap[env] = pair<string, string>("", value);
        }
    } else {
        envMap[env].second = value;
    }
}

string NPShell::getEnv(string env) {
    auto envRaw = getenv(env.c_str());
    if (envMap.find(env) == envMap.end()) {
        if (envRaw != nullptr) {
            envMap[env] = pair<string, string>(string(envRaw), string(envRaw));
        } else {
            envMap[env] = pair<string, string>("", "");
        }
    }
    return envMap[env].second;
}

void NPShell::loadEnv() {
    for (auto envPair : envMap) {
        setenv(envPair.first.c_str(), envPair.second.second.c_str(), 1);
    }
}
void NPShell::resetEnv() {
    for (auto envPair : envMap) {
        setenv(envPair.first.c_str(), envPair.second.first.c_str(), 1);
    }
}

void NPShell::setExit() { exitFlag = true; }

bool NPShell::getExit() { return exitFlag; }