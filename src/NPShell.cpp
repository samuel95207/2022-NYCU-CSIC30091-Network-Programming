#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
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

#ifndef _UTIL_H_
#define _UTIL_H_
#include "Util.h"
#endif


using namespace std;

NPShell::NPShell() {
    signal(SIGCHLD, NPShell::childSignalHandler);
    BuildinCommand::execute(*this, "setenv", {vector<string>({"PATH", "bin:."})});
}

void NPShell::run() {
    string commandRaw;
    while (cout << symbol && getline(cin, commandRaw)) {
        auto parseResult = Parser::parse(commandRaw);
        // Parser::printParseResult(parseResult);

        pipeManager.newSession();

        for (int i = 0; i < int(parseResult.commands.size()); i++) {
            auto command = parseResult.commands[i].first;
            auto args = parseResult.commands[i].second;
            auto prevOperator = (i != 0 ? parseResult.operators[i - 1] : "");
            auto nextOperator = (i != int(parseResult.operators.size()) ? parseResult.operators[i] : "");


            // Filter buildin commands
            if (BuildinCommand::isBuildinCommand(command)) {
                BuildinCommand::execute(*this, command, args);
                continue;
            }

            if (nextOperator == "|") {
                executeForkedCommand(command, args);
            } else if (nextOperator == ">") {
                if (i + 1 >= int(parseResult.commands.size())) {
                    cerr << "Error! Filename cannot be empty." << endl;
                }
                string filename = parseResult.commands[i + 1].first;
                executeForkedCommand(command, args, true, filename);
                break;
            } else if (nextOperator[0] == '|') {
            } else if (nextOperator[0] == '!') {
            } else {
                executeForkedCommand(command, args, true);
            }
        }

        if (exitFlag) {
            break;
        }
    }
}

bool NPShell::executeForkedCommand(const std::string& command, const std::vector<std::string>& args, bool pipeEnd,
                                   std::string outFilename) {
    bool pipeStatus = pipeManager.rootPipeHandler();

    if (!pipeStatus) {
        cerr << "Pipe error!" << endl;
        return false;
    }


    pid_t pid = fork();

    if (pid == -1) {
        cerr << "Fork error!" << endl;
        return false;

    } else if (pid > 0) {
        // Parent Process
        pipeManager.parentPipeHandler(pipeEnd, outFilename);

        if (pipeEnd) {
            int status;
            waitpid(pid, &status, 0);
        }

    } else {
        // Children Process
        pipeManager.childPipeHandler(pipeEnd, outFilename);

        execvp(command.c_str(), (char**)Util::createArgv(command, args));

        // cerr << "errno = " << errno << endl;
        if (errno == 13 || errno == 2) {
            cerr << "Unknown command: [" << command << "]." << endl;
        }
        exit(0);
    }

    return true;
}


void NPShell::setExit() { exitFlag = true; }


void NPShell::childSignalHandler(int signum) {
    int status;
    wait(&status);
}
