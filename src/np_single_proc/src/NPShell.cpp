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

#ifndef _SINGLE_PROC_SERVER_H_
#define _SINGLE_PROC_SERVER_H_
#include "SingleProcServer.h"
#endif

using namespace std;

NPShell::NPShell() {
    signal(SIGCHLD, NPShell::childSignalHandler);
    historyFilePath = string(getpwuid(getuid())->pw_dir) + "/.npshell_history";
    // cout << historyFilePath << endl;
}

void NPShell::execute(string commandRaw, SingleProcServer &server, int fd) {
    loadEnv();
    auto parseResult = Parser::parse(commandRaw);
    // Parser::printParseResult(parseResult);

    ofstream outfile(historyFilePath.c_str(), ios::app);
    outfile << commandRaw << endl;
    outfile.close();

    for (int i = 0; i < int(parseResult.commands.size()); i++) {
        auto command = parseResult.commands[i].first;
        auto args = parseResult.commands[i].second;
        auto prevOperator = (i != 0 ? parseResult.operators[i - 1] : "");
        auto nextOperator = (i < int(parseResult.operators.size()) ? parseResult.operators[i] : "");
        auto nextNextOperator = (i + 1 < int(parseResult.operators.size()) ? parseResult.operators[i + 1] : "");


        bool doubleUserPipeFlag = false;
        int toUserId;
        int fromUserId;

        // Filter buildin commands
        if (BuildinCommand::isBuildinCommand(command)) {
            BuildinCommand::execute(*this, server, fd, command, args);

            pipeManager.newSession();
            continue;
        }

        if (nextOperator == "|") {
            // To next normal pipe
            executeForkedCommand(command, args, PipeMode::NORMAL_PIPE);
        } else if (nextOperator == ">") {
            // To file
            if (i + 1 >= int(parseResult.commands.size())) {
                cerr << "Error! Filename cannot be empty." << endl;
                break;
            }
            string filename = parseResult.commands[i + 1].first;
            executeForkedCommand(command, args, PipeMode::FILE_OUTPUT, filename);

            pipeManager.newSession();
            break;
        } else if (nextOperator[0] == '|') {
            // To numbered pipe
            int count;
            sscanf(nextOperator.c_str(), "|%d", &count);

            pipeManager.addNumberedPipe(count);
            executeForkedCommand(command, args, PipeMode::NUMBERED_PIPE);

            pipeManager.newSession();
        } else if (nextOperator[0] == '!') {
            // To STDERR numbered pipe
            int count;
            sscanf(nextOperator.c_str(), "!%d", &count);

            pipeManager.addNumberedPipe(count);
            executeForkedCommand(command, args, PipeMode::NUMBERED_PIPE_STDERR);

            pipeManager.newSession();
        } else if (nextOperator[0] == '>') {
            sscanf(nextOperator.c_str(), ">%d", &toUserId);
            if (nextNextOperator != "<" && nextNextOperator[0] == '<') {
                doubleUserPipeFlag = true;
                sscanf(nextNextOperator.c_str(), "<%d", &fromUserId);

            } else {
                if (server.userManager.getUserById(toUserId) == nullptr) {
                    cerr << "*** Error: user #" << toUserId << " does not exist yet. ***" << endl;
                } else {
                    User *me = server.userManager.getUserByFd(fd);
                    User *toUser = server.userManager.getUserById(toUserId);
                    if (!pipeManager.addUserPipe(me->id, toUserId)) {
                        cerr << "*** Error: the pipe #" << me->id << "->#" << toUserId << " already exists. ***"
                             << endl;
                    } else {
                        string message = "*** " + (me->name == "" ? "(no name)" : me->name) + " (#" +
                                         to_string(me->id) + ") just piped '" + commandRaw + "' to " +
                                         (toUser->name == "" ? "(no name)" : toUser->name) + " (#" +
                                         to_string(toUserId) + ") ***\n";
                        server.broadcast(message);
                    }
                }
                executeForkedCommand(command, args, PipeMode::USER_PIPE_OUT);
                pipeManager.newSession();

                break;
            }
        } else if (nextOperator[0] == '<') {
            sscanf(nextOperator.c_str(), "<%d", &fromUserId);
            if (nextNextOperator != ">" && nextNextOperator[0] == '>') {
                doubleUserPipeFlag = true;
                sscanf(nextNextOperator.c_str(), ">%d", &toUserId);
            } else {
                if (server.userManager.getUserById(fromUserId) == nullptr) {
                    cerr << "*** Error: user #" << fromUserId << " does not exist yet. ***" << endl;
                } else {
                    User *fromUser = server.userManager.getUserById(fromUserId);
                    User *me = server.userManager.getUserByFd(fd);

                    if (!pipeManager.loadUserPipe(fromUserId, me->id)) {
                        cerr << "*** Error: the pipe #" << me->id << "->#" << fromUserId << " does not exist yet. ***"
                             << endl;
                    } else {
                        string message = "*** " + (me->name == "" ? "(no name)" : me->name) + " (#" +
                                         to_string(me->id) + ") just received from " +
                                         (fromUser->name == "" ? "(no name)" : fromUser->name) + " (#" +
                                         to_string(fromUserId) + ") by '" + commandRaw + "' ***\n";
                        server.broadcast(message);
                    }
                }
                executeForkedCommand(command, args, PipeMode::USER_PIPE_IN);
                pipeManager.newSession();

                break;
            }
        } else {
            // To console
            executeForkedCommand(command, args, PipeMode::CONSOLE_OUTPUT);

            pipeManager.newSession();
        }

        if (doubleUserPipeFlag) {
            User *me = server.userManager.getUserByFd(fd);
            User *fromUser = server.userManager.getUserById(fromUserId);
            User *toUser = server.userManager.getUserById(toUserId);

            cout << fromUserId << " " << toUserId << endl;

            if (server.userManager.getUserById(fromUserId) == nullptr) {
                cerr << "*** Error: user #" << fromUserId << " does not exist yet. ***" << endl;
            } else {
                if (!pipeManager.loadUserPipe(fromUserId, me->id)) {
                    cerr << "*** Error: the pipe #" << fromUserId << "->#" << me->id << " does not exist yet. ***"
                         << endl;
                } else {
                    string message = "*** " + (me->name == "" ? "(no name)" : me->name) + " (#" + to_string(me->id) +
                                     ") just received from " + (fromUser->name == "" ? "(no name)" : fromUser->name) +
                                     " (#" + to_string(fromUserId) + ") by '" + commandRaw + "' ***\n";
                    server.broadcast(message);
                }
            }

            if (server.userManager.getUserById(toUserId) == nullptr) {
                cerr << "*** Error: user #" << toUserId << " does not exist yet. ***" << endl;
            } else {
                if (!pipeManager.addUserPipe(me->id, toUserId)) {
                    cerr << "*** Error: the pipe #" << me->id << "->#" << toUserId << " already exists. ***" << endl;
                } else {
                    string message = "*** " + (me->name == "" ? "(no name)" : me->name) + " (#" + to_string(me->id) +
                                     ") just piped '" + commandRaw + "' to " +
                                     (toUser->name == "" ? "(no name)" : toUser->name) + " (#" + to_string(toUserId) +
                                     ") ***\n";
                    server.broadcast(message);
                }
            }

            executeForkedCommand(command, args, PipeMode::USER_PIPE_BOTH);
            pipeManager.newSession();

            break;
        }
    }

    resetEnv();
}

string NPShell::getSymbol() { return symbol; }

bool NPShell::executeForkedCommand(const std::string &command, const std::vector<std::string> &args, PipeMode pipeMode,
                                   std::string outFilename) {
    if (!pipeManager.rootPipeHandler(pipeMode, outFilename)) {
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
        if (!pipeManager.parentPipeHandler(pipeMode, outFilename)) {
            cerr << "Pipe error!" << endl;
            return false;
        }

        if (pipeMode == PipeMode::CONSOLE_OUTPUT || pipeMode == PipeMode::FILE_OUTPUT) {
            int status;
            waitpid(pid, &status, 0);
        }
    } else {
        // Children Process
        if (!pipeManager.childPipeHandler(pipeMode, outFilename)) {
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

void NPShell::childSignalHandler(int signum) {
    int status;
    wait(&status);
}
