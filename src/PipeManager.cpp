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

bool PipeManager::rootPipeHandler(NumberedPipe *numberedPipe, bool pipeEnd, std::string outFilename) {
    if (!pipeEnd) {
        if (numberedPipe != nullptr) {
            // cerr << "Add numberedPipe " << numberedPipe->pipe[READ] << " " << numberedPipe->pipe[WRITE] << endl;
            newPipe[READ] = numberedPipe->pipe[READ];
            newPipe[WRITE] = numberedPipe->pipe[WRITE];
        } else {
            if (pipe(newPipe)) {
                return false;
            }
        }
    }

    int *currentNumberedPipe = getCurrentNumberedPipe();
    if (currentNumberedPipe != nullptr) {
        // cerr << "currentNumberedPipe " << currentNumberedPipe[READ] << " " << currentNumberedPipe[WRITE] << endl;
        currentPipe[READ] = currentNumberedPipe[READ];
        currentPipe[WRITE] = currentNumberedPipe[WRITE];
        delete currentNumberedPipe;
    }

    return true;
}

bool PipeManager::parentPipeHandler(NumberedPipe *numberedPipe, bool pipeEnd, std::string outFilename) {
    if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
        close(currentPipe[READ]);
        close(currentPipe[WRITE]);
    }

    if (!pipeEnd) {
        if (numberedPipe != nullptr) {
            dup2(numberedPipe->pipe[READ], newPipe[READ]);
        } else {
            currentPipe[READ] = newPipe[READ];
            currentPipe[WRITE] = newPipe[WRITE];
        }
    }

    return true;
}

bool PipeManager::childPipeHandler(NumberedPipe *numberedPipe, bool pipeEnd, std::string outFilename) {
    // cerr << "child currentPipe " << currentPipe[READ] << " " << currentPipe[WRITE] << endl;

    if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
        dup2(currentPipe[READ], fileno(stdin));
        close(currentPipe[READ]);
        close(currentPipe[WRITE]);
    }

    if (!pipeEnd) {
        close(newPipe[READ]);
        dup2(newPipe[WRITE], fileno(stdout));
        if(numberedPipe != nullptr && numberedPipe->includeStderr){
            dup2(newPipe[WRITE], fileno(stderr));
        }
        close(newPipe[WRITE]);
    } else {
        if (outFilename != "") {
            int permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
            int outfile = open(outFilename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, permission);
            dup2(outfile, fileno(stdout));
        }
    }

    return true;
}

NumberedPipe PipeManager::addNumberedPipe(int countIn, bool includeStderr) {
    NumberedPipe numberedPipe;

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

    numberedPipe.pipe[READ] = findedPipe[READ];
    numberedPipe.pipe[WRITE] = findedPipe[WRITE];
    numberedPipe.includeStderr = includeStderr;

    return numberedPipe;
}

int *PipeManager::getCurrentNumberedPipe() {
    int *findedPipe = new int[2];
    auto findedPipeIter = countPipeMap.find(count);
    if (findedPipeIter == countPipeMap.end()) {
        return nullptr;
    } else {
        findedPipe[READ] = findedPipeIter->second.first;
        findedPipe[WRITE] = findedPipeIter->second.second;
        countPipeMap.erase(count);
    }
    return findedPipe;
}

void PipeManager::reduceNumberedPipesCount() { count++; }
