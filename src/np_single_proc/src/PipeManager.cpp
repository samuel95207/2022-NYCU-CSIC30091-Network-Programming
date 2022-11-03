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
#include <utility>

using namespace std;

map<pair<int, int>, pair<int, int>> PipeManager::userPipeMap;


PipeManager::PipeManager() { newSession(); }

bool PipeManager::newSession() {
    currentPipe[READ] = 0;
    currentPipe[WRITE] = 0;
    newPipe[READ] = 0;
    newPipe[WRITE] = 0;
    newNumberedPipe[READ] = 0;
    newNumberedPipe[WRITE] = 0;
    currentNumberedPipe[READ] = 0;
    currentNumberedPipe[WRITE] = 0;

    newUserPipe[READ] = 0;
    newUserPipe[WRITE] = 0;
    currentUserPipe[READ] = 0;
    currentUserPipe[WRITE] = 0;


    reduceNumberedPipesCount();
    loadCurrentNumberedPipe();

    return true;
}

bool PipeManager::rootPipeHandler(PipeMode pipeMode, PipeMode pipeMode2, std::string outFilename) {
    // cout << "currentPipe " << endl;
    // cout << currentPipe[READ] << " " << currentPipe[WRITE] << endl;
    // cout << count << endl;


    // Check if current numbered pipe exist
    if (pipeMode == PipeMode::USER_PIPE_IN || pipeMode == PipeMode::USER_PIPE_BOTH) {
        if (currentUserPipe[READ] == 0 || currentUserPipe[WRITE] == 0) {
            currentPipe[READ] = fileno(fopen("/dev/null", "r"));
            currentPipe[WRITE] = fileno(fopen("/dev/null", "w"));
        } else {
            // cout << "currentUserPipe " << currentUserPipe[READ] << " " << currentUserPipe[WRITE] << endl;

            currentPipe[READ] = currentUserPipe[READ];
            currentPipe[WRITE] = currentUserPipe[WRITE];
        }

        currentUserPipe[READ] = 0;
        currentUserPipe[WRITE] = 0;


    } else if (currentNumberedPipe[READ] != 0 && currentNumberedPipe[WRITE] != 0) {
        // cout << "currentNumberedPipe " << currentNumberedPipe[READ] << " " << currentNumberedPipe[WRITE] << endl;

        currentPipe[READ] = currentNumberedPipe[READ];
        currentPipe[WRITE] = currentNumberedPipe[WRITE];


        currentNumberedPipe[READ] = 0;
        currentNumberedPipe[WRITE] = 0;
    }

    // Check if new numbered pipe is created
    if (pipeMode == PipeMode::USER_PIPE_OUT || pipeMode == PipeMode::USER_PIPE_BOTH) {
        if (newUserPipe[READ] == 0 || newUserPipe[WRITE] == 0) {
            newPipe[READ] = fileno(fopen("/dev/null", "r"));
            newPipe[WRITE] = fileno(fopen("/dev/null", "w"));
        } else {
            newPipe[READ] = newUserPipe[READ];
            newPipe[WRITE] = newUserPipe[WRITE];
        }

    } else if (pipeMode == PipeMode::NUMBERED_PIPE || pipeMode == PipeMode::NUMBERED_PIPE_STDERR ||
               pipeMode2 == PipeMode::NUMBERED_PIPE || pipeMode2 == PipeMode::NUMBERED_PIPE_STDERR) {
        // cerr << "Add numberedPipe " << numberedPipe->pipe[READ] << " " << numberedPipe->pipe[WRITE] << endl;
        newPipe[READ] = newNumberedPipe[READ];
        newPipe[WRITE] = newNumberedPipe[WRITE];

    } else if (pipeMode == PipeMode::NORMAL_PIPE || pipeMode2 == PipeMode::NORMAL_PIPE) {
        if (pipe(newPipe)) {
            return false;
        }
        // cout << "new pipe" << endl;
        // cout << newPipe[READ] << " " << newPipe[WRITE] << endl;
    }

    // cerr << "\tCurrentPipe " << currentPipe[READ] << " " << currentPipe[WRITE] << endl;
    // cerr << "\tNewPipe " << newPipe[READ] << " " << newPipe[WRITE] << endl;


    return true;
}

bool PipeManager::parentPipeHandler(PipeMode pipeMode, PipeMode pipeMode2, std::string outFilename) {
    if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
        close(currentPipe[READ]);
        close(currentPipe[WRITE]);
    }

    if (pipeMode == PipeMode::NORMAL_PIPE || pipeMode2 == PipeMode::NORMAL_PIPE) {
        currentPipe[READ] = newPipe[READ];
        currentPipe[WRITE] = newPipe[WRITE];

        // cout << "Copy current pipe" << endl;
        // cout << currentPipe[READ] << " " << currentPipe[WRITE] << endl;
    }

    newPipe[READ] = 0;
    newPipe[WRITE] = 0;

    return true;
}

bool PipeManager::childPipeHandler(PipeMode pipeMode, PipeMode pipeMode2, std::string outFilename) {
    // cerr << "child currentPipe " << currentPipe[READ] << " " << currentPipe[WRITE] << endl;

    // Direct pipe from previous command to STDIN of current command
    if (currentPipe[READ] != 0 && currentPipe[WRITE] != 0) {
        close(currentPipe[WRITE]);
        dup2(currentPipe[READ], fileno(stdin));
        close(currentPipe[READ]);

        // cout << "prev pipe exist" << endl;
        // cout << currentPipe[READ] << " " << currentPipe[WRITE] << endl;
    }


    // Direct pipe from current command to output file
    if (pipeMode == PipeMode::FILE_OUTPUT || pipeMode2 == PipeMode::FILE_OUTPUT) {
        int permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        int outfile = open(outFilename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, permission);
        dup2(outfile, fileno(stdout));

        return true;
    }

    // Direct STDOUT or STDERR from current command to new pipe
    if (pipeMode == PipeMode::NORMAL_PIPE || pipeMode == PipeMode::NUMBERED_PIPE ||
        pipeMode == PipeMode::NUMBERED_PIPE_STDERR || pipeMode == PipeMode::USER_PIPE_OUT ||
        pipeMode == PipeMode::USER_PIPE_BOTH || pipeMode2 == PipeMode::NORMAL_PIPE ||
        pipeMode2 == PipeMode::NUMBERED_PIPE || pipeMode2 == PipeMode::NUMBERED_PIPE_STDERR) {
        close(newPipe[READ]);
        dup2(newPipe[WRITE], fileno(stdout));
        if (pipeMode == PipeMode::NUMBERED_PIPE_STDERR || pipeMode2 == PipeMode::NUMBERED_PIPE_STDERR) {
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

    // cout << "Add numbered pipe" << endl;
    // cout << idx << endl;

    int findedPipe[2] = {0, 0};
    if (findedPipeIter == countPipeMap.end()) {
        if (pipe(findedPipe)) {
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

bool PipeManager::addUserPipe(int fromId, int toId) {
    pair<int, int> key = pair<int, int>(fromId, toId);

    newUserPipe[READ] = 0;
    newUserPipe[WRITE] = 0;

    if (userPipeMap.find(key) != userPipeMap.end()) {
        return false;
    }

    if (pipe(newUserPipe)) {
        cerr << "Error! Pipe error." << endl;
        return false;
    }

    userPipeMap[key] = pair<int, int>(newUserPipe[READ], newUserPipe[WRITE]);


    return true;
}


bool PipeManager::loadUserPipe(int fromId, int toId) {
    pair<int, int> key = pair<int, int>(fromId, toId);

    currentUserPipe[READ] = 0;
    currentUserPipe[WRITE] = 0;

    auto findedPipeIter = userPipeMap.find(key);
    if (findedPipeIter == userPipeMap.end()) {
        return false;
    }


    currentUserPipe[READ] = findedPipeIter->second.first;
    currentUserPipe[WRITE] = findedPipeIter->second.second;

    userPipeMap.erase(key);

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
    countPipeMap.erase(count);
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
