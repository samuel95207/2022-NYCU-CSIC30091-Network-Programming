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
    newPipe[READ] = 0;
    newPipe[WRITE] = 0;
    currentNumberedPipe[READ] = 0;
    currentNumberedPipe[WRITE] = 0;
    newNumberedPipe[READ] = 0;
    newNumberedPipe[WRITE] = 0;
}

bool PipeManager::newSession() {
    currentPipe[READ] = 0;
    currentPipe[WRITE] = 0;
    newPipe[READ] = 0;
    newPipe[WRITE] = 0;
    newNumberedPipe[READ] = 0;
    newNumberedPipe[WRITE] = 0;
    currentNumberedPipe[READ] = 0;
    currentNumberedPipe[WRITE] = 0;

    reduceNumberedPipesCount();
    loadCurrentNumberedPipe();

    return true;
}

bool PipeManager::rootPipeHandler(PipeMode pipeMode, std::string outFilename) {
    // Check if current numbered pipe exist
    if (currentNumberedPipe[READ] != 0 && currentNumberedPipe[WRITE] != 0) {
        // cerr << "currentNumberedPipe " << currentNumberedPipe[READ] << " " << currentNumberedPipe[WRITE] << endl;
        currentPipe[READ] = currentNumberedPipe[READ];
        currentPipe[WRITE] = currentNumberedPipe[WRITE];
    }

    // Check if new numbered pipe is created
    if (pipeMode == PipeMode::NUMBERED_PIPE || pipeMode == PipeMode::NUMBERED_PIPE_STDERR) {
        // cerr << "Add numberedPipe " << numberedPipe->pipe[READ] << " " << numberedPipe->pipe[WRITE] << endl;
        if (newNumberedPipe[READ] != 0 && newNumberedPipe[WRITE] != 0) {
            newPipe[READ] = newNumberedPipe[READ];
            newPipe[WRITE] = newNumberedPipe[WRITE];
        }

    } else if (pipeMode == PipeMode::NORMAL_PIPE) {
        if (pipe(newPipe)) {
            return false;
        }
    }


    return true;
}

bool PipeManager::parentPipeHandler(PipeMode pipeMode, std::string outFilename) {
    if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
        close(currentPipe[READ]);
        close(currentPipe[WRITE]);
    }

    if (pipeMode == PipeMode::NUMBERED_PIPE || pipeMode == PipeMode::NUMBERED_PIPE_STDERR) {
        dup2(newNumberedPipe[READ], newPipe[READ]);
    } else if (pipeMode == PipeMode::NORMAL_PIPE) {
        currentPipe[READ] = newPipe[READ];
        currentPipe[WRITE] = newPipe[WRITE];
    }

    return true;
}

bool PipeManager::childPipeHandler(PipeMode pipeMode, std::string outFilename) {
    // cerr << "child currentPipe " << currentPipe[READ] << " " << currentPipe[WRITE] << endl;

    // Direct pipe from previous command to STDIN of current command
    if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
        close(currentPipe[WRITE]);
        dup2(currentPipe[READ], fileno(stdin));
        close(currentPipe[READ]);
    }


    // Direct pipe from current command to output file
    if (pipeMode == PipeMode::FILE_OUTPUT) {
        int permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        int outfile = open(outFilename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, permission);
        dup2(outfile, fileno(stdout));

        return true;
    }

    // Direct STDOUT or STDERR from current command to new pipe
    if (pipeMode == PipeMode::NORMAL_PIPE || pipeMode == PipeMode::NUMBERED_PIPE ||
        pipeMode == PipeMode::NUMBERED_PIPE_STDERR) {
        close(newPipe[READ]);
        dup2(newPipe[WRITE], fileno(stdout));
        if (pipeMode == PipeMode::NUMBERED_PIPE_STDERR) {
            dup2(newPipe[WRITE], fileno(stderr));
        }
        close(newPipe[WRITE]);

        return true;
    }

    return true;
}

bool PipeManager::addNumberedPipe(int countIn) {
    int idx = count + countIn;
    auto findedPipeIter = countPipeMap.find(idx);

    int findedPipe[2] = {0, 0};
    if (findedPipeIter == countPipeMap.end()) {
        if(pipe(findedPipe)){
            return false;
        }
        countPipeMap[idx] = pair<int, int>(findedPipe[READ], findedPipe[WRITE]);
    } else {
        findedPipe[READ] = findedPipeIter->second.first;
        findedPipe[WRITE] = findedPipeIter->second.second;
    }

    newNumberedPipe[READ] = findedPipe[READ];
    newNumberedPipe[WRITE] = findedPipe[WRITE];

    return true;
}

void PipeManager::loadCurrentNumberedPipe() {
    auto findedPipeIter = countPipeMap.find(count);
    if (findedPipeIter == countPipeMap.end()) {
        currentNumberedPipe[READ] = 0;
        currentNumberedPipe[WRITE] = 0;
        return;
    }
    currentNumberedPipe[READ] = findedPipeIter->second.first;
    currentNumberedPipe[WRITE] = findedPipeIter->second.second;
}

void PipeManager::reduceNumberedPipesCount() {
    countPipeMap.erase(count);
    count++;
    // printCountPipeMap();
}

void PipeManager::printCountPipeMap() {
    for (auto keyValuePair : countPipeMap) {
        cout << keyValuePair.first << ": " << keyValuePair.second.first << " " << keyValuePair.second.second << endl;
    }
}
