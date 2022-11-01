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

using namespace std;


NPShell::NPShell() {
    signal(SIGCHLD, NPShell::childSignalHandler);
    historyFilePath = string(getpwuid(getuid())->pw_dir) + "/.npshell_history";
    // cout << historyFilePath << endl;
}

void NPShell::execute(string commandRaw, SingleProcServer &server, int fd) {
    auto parseResult = Parser::parse(commandRaw);
    // Parser::printParseResult(parseResult);


    ofstream outfile(historyFilePath.c_str(), ios::app);
    outfile << commandRaw << endl;
    outfile.close();

    for (int i = 0; i < int(parseResult.commands.size()); i++) {
        auto command = parseResult.commands[i].first;
        auto args = parseResult.commands[i].second;
        auto prevOperator = (i != 0 ? parseResult.operators[i - 1] : "");
        auto nextOperator = (i != int(parseResult.operators.size()) ? parseResult.operators[i] : "");


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

        } else {
            // To console
            executeForkedCommand(command, args, PipeMode::CONSOLE_OUTPUT);

            pipeManager.newSession();
        }
    }
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

void NPShell::setExit() { exitFlag = true; }

bool NPShell::getExit() { return exitFlag; }


void NPShell::childSignalHandler(int signum) {
    int status;
    wait(&status);
}
