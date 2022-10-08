#ifndef _PIPE_MANAGER_H_
#define _PIPE_MANAGER_H_
#include "PipeManager.h"
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>

using namespace std;

PipeManager::PipeManager() {
    currentPipe[READ] = 0;
    currentPipe[WRITE] = 0;
}

bool PipeManager::newSession() {
    currentPipe[READ] = 0;
    currentPipe[WRITE] = 0;

    return true;
}

bool PipeManager::rootPipeHandler(PipeMode pipeMode, std::string outFilename) {
    if (pipeMode != PipeMode::CONSOLE_OUTPUT) {
        if (pipeMode == PipeMode::NUMBERED_PIPE || pipeMode == PipeMode::NUMBERED_PIPE_STDERR) {
            // cerr << "Add numberedPipe " << numberedPipe->pipe[READ] << " " << numberedPipe->pipe[WRITE] << endl;

            newPipe[READ] = newNumberedPipe[READ];
            newPipe[WRITE] = newNumberedPipe[WRITE];
        } else {
            if (pipe(newPipe)) {
                return false;
            }
        }
    }

    auto currentNumberedPipe = getCurrentNumberedPipe();
    if (currentNumberedPipe.first != -1 && currentNumberedPipe.second != -1) {
        // cerr << "currentNumberedPipe " << currentNumberedPipe[READ] << " " << currentNumberedPipe[WRITE] << endl;
        currentPipe[READ] = currentNumberedPipe.first;
        currentPipe[WRITE] = currentNumberedPipe.second;
    }

    return true;
}

bool PipeManager::parentPipeHandler(PipeMode pipeMode, std::string outFilename) {
    if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
        close(currentPipe[READ]);
        close(currentPipe[WRITE]);
    }

    if (pipeMode != PipeMode::CONSOLE_OUTPUT) {
        if (pipeMode == PipeMode::NUMBERED_PIPE || pipeMode == PipeMode::NUMBERED_PIPE_STDERR) {
            dup2(newNumberedPipe[READ], newPipe[READ]);
        } else {
            currentPipe[READ] = newPipe[READ];
            currentPipe[WRITE] = newPipe[WRITE];
        }
    }

    return true;
}

bool PipeManager::childPipeHandler(PipeMode pipeMode, std::string outFilename) {
    // cerr << "child currentPipe " << currentPipe[READ] << " " << currentPipe[WRITE] << endl;

    if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
        dup2(currentPipe[READ], fileno(stdin));
        close(currentPipe[READ]);
        close(currentPipe[WRITE]);
    }



    if (pipeMode == PipeMode::FILE_OUTPUT) {
        int permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        int outfile = open(outFilename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, permission);
        dup2(outfile, fileno(stdout));
    } else if (pipeMode != PipeMode::CONSOLE_OUTPUT) {
        close(newPipe[READ]);
        dup2(newPipe[WRITE], fileno(stdout));
        if (pipeMode == PipeMode::NUMBERED_PIPE_STDERR) {
            dup2(newPipe[WRITE], fileno(stderr));
        }
        close(newPipe[WRITE]);
    }

    return true;
}

void PipeManager::addNumberedPipe(int countIn) {
    int idx = count + countIn;
    auto findedPipeIter = countPipeMap.find(idx);

    int findedPipe[2] = {0, 0};
    if (findedPipeIter == countPipeMap.end()) {
        pipe(findedPipe);
        countPipeMap[idx] = pair<int, int>(findedPipe[READ], findedPipe[WRITE]);
    } else {
        findedPipe[READ] = findedPipeIter->second.first;
        findedPipe[WRITE] = findedPipeIter->second.second;
    }

    newNumberedPipe[READ] = findedPipe[READ];
    newNumberedPipe[WRITE] = findedPipe[WRITE];
}

pair<int, int> PipeManager::getCurrentNumberedPipe() {
    auto findedPipeIter = countPipeMap.find(count);
    if (findedPipeIter == countPipeMap.end()) {
        return pair<int, int>(-1, -1);
    }
    return findedPipeIter->second;
}

void PipeManager::reduceNumberedPipesCount() {
    countPipeMap.erase(count);
    count++;
}
