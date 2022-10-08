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
    pair<int, int> currentPipe(0, 0);
    currentPipes.push_back(currentPipe);
}

bool PipeManager::newSession() {
    currentPipes.clear();

    pair<int, int> currentPipe(0, 0);
    currentPipes.push_back(currentPipe);

    return true;
}


bool PipeManager::rootPipeHandler(bool pipeEnd, std::string outFilename) {
    bool clearCurrentPipesFlag = true;
    for (int i = 0; i < int(numberedPipes.size()); i++) {
        if (numberedPipes[i].count == 0) {
            // cerr << "numberedPipes " << i << " = 0" << endl;
            // cerr << numberedPipes[i].pipe[READ] << " " << numberedPipes[i].pipe[WRITE] << endl;

            if (clearCurrentPipesFlag) {
                currentPipes.clear();
                clearCurrentPipesFlag = false;
            }

            pair<int, int> currentPipe(numberedPipes[i].pipe[READ], numberedPipes[i].pipe[WRITE]);
            currentPipes.push_back(currentPipe);
        }
    }

    // cerr << "currentPipes size = " << currentPipes.size() << endl;
    // for (auto currentPipe : currentPipes) {
        // cerr << currentPipe.first << " " << currentPipe.second << endl;
    // }

    numberedPipes.erase(
        std::remove_if(numberedPipes.begin(), numberedPipes.end(),
                       [&](const NumberedPipe numberedPipe) -> bool { return numberedPipe.count == 0; }),
        numberedPipes.end());

    if (!pipeEnd) {
        if (pipe(newPipe)) {
            return false;
        }
    }

    return true;
}

bool PipeManager::parentPipeHandler(bool pipeEnd, std::string outFilename) {
    for (auto currentPipePair : currentPipes) {
        int currentPipe[2] = {currentPipePair.first, currentPipePair.second};

        if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
            close(currentPipe[READ]);
            close(currentPipe[WRITE]);
        }
    }

    if (!pipeEnd) {
        currentPipes[0].first = newPipe[READ];
        currentPipes[0].second = newPipe[WRITE];
    }

    return true;
}

bool PipeManager::childPipeHandler(bool pipeEnd, std::string outFilename) {
    for (auto currentPipePair : currentPipes) {
        int currentPipe[2] = {currentPipePair.first, currentPipePair.second};

        if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
            cerr << currentPipe[READ] << " " << currentPipe[WRITE] << " to stdin" << endl;

            dup2(currentPipe[READ], fileno(stdin));
            // close(currentPipe[READ]);
            // close(currentPipe[WRITE]);
        }
    }



    if (!pipeEnd) {
        close(newPipe[READ]);
        dup2(newPipe[WRITE], fileno(stdout));
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


bool PipeManager::addNumberedPipe(int count, bool includeStderr) {
    NumberedPipe newNumberedPipe;
    newNumberedPipe.count = count;
    newNumberedPipe.pipe[READ] = currentPipes[0].first;
    newNumberedPipe.pipe[WRITE] = currentPipes[0].second;
    newNumberedPipe.includeStderr = includeStderr;

    numberedPipes.push_back(newNumberedPipe);

    return true;
}


void PipeManager::reduceNumberedPipesCount() {
    for (int i = 0; i < int(numberedPipes.size()); i++) {
        numberedPipes[i].count--;
    }
}
