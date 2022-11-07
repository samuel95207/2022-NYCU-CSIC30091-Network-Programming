#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#ifndef _PIPE_MANAGER_H_
#define _PIPE_MANAGER_H_
#include "PipeManager.h"
#endif

#ifndef _MESSAGE_MANAGER_H_
#define _MESSAGE_MANAGER_H_
#include "MessageManager.h"
#endif

#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include "UserManager.h"
#endif



using namespace std;

const string PipeManager::fifoPath = "./user_pipe";


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

    newUserPipe = 0;
    currentUserPipe = 0;


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
        if (currentUserPipe <= 0) {
            currentPipe[READ] = fileno(fopen("/dev/null", "r"));
            currentPipe[WRITE] = fileno(fopen("/dev/null", "w"));
        } else {
            // cout << "currentUserPipe " << currentUserPipe[READ] << " " << currentUserPipe[WRITE] << endl;

            currentPipe[READ] = currentUserPipe;
            currentPipe[WRITE] = fileno(fopen("/dev/null", "w"));
        }

        currentUserPipe = 0;


    } else if (currentNumberedPipe[READ] != 0 && currentNumberedPipe[WRITE] != 0) {
        // cout << "currentNumberedPipe " << currentNumberedPipe[READ] << " " << currentNumberedPipe[WRITE] << endl;

        currentPipe[READ] = currentNumberedPipe[READ];
        currentPipe[WRITE] = currentNumberedPipe[WRITE];


        currentNumberedPipe[READ] = 0;
        currentNumberedPipe[WRITE] = 0;
    }

    // Check if new numbered pipe is created
    if (pipeMode == PipeMode::USER_PIPE_OUT || pipeMode == PipeMode::USER_PIPE_BOTH) {
        if (newUserPipe <= 0) {
            newPipe[READ] = fileno(fopen("/dev/null", "r"));
            newPipe[WRITE] = fileno(fopen("/dev/null", "w"));
        } else {
            newPipe[READ] = fileno(fopen("/dev/null", "r"));
            newPipe[WRITE] = newUserPipe;
        }

    } else if (pipeMode == PipeMode::NUMBERED_PIPE || pipeMode == PipeMode::NUMBERED_PIPE_STDERR ||
               pipeMode2 == PipeMode::NUMBERED_PIPE || pipeMode2 == PipeMode::NUMBERED_PIPE_STDERR) {
        // cerr << "Add numberedPipe " << numberedPipe->pipe[READ] << " " << numberedPipe->pipe[WRITE] << endl;
        newPipe[READ] = newNumberedPipe[READ];
        newPipe[WRITE] = newNumberedPipe[WRITE];

    } else if (pipeMode == PipeMode::NORMAL_PIPE || pipeMode2 == PipeMode::NORMAL_PIPE) {
        int pipeResult;
        do {
            pipeResult = pipe(newPipe);
        } while (pipeResult < 0);

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

    if (pipeMode == PipeMode::USER_PIPE_OUT || pipeMode == PipeMode::USER_PIPE_BOTH) {
        if (newUserPipe > 0) {
            close(dummyReadFd);
            close(newUserPipe);
        }
        dummyReadFd = 0;
        newUserPipe = 0;
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
        int pipeResult;
        do {
            pipeResult = pipe(findedPipe);
        } while (pipeResult < 0);

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
    string path = "./user_pipe/" + to_string(fromId) + "_" + to_string(toId);

    if (mknod(path.c_str(), S_IFIFO | FIFO_PERMS, 0) < 0) {
        // cerr << "mknod() error! " << strerror(errno) << " " << errno << endl;
        return false;
    }
    dummyReadFd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    newUserPipe = open(path.c_str(), O_WRONLY);
    if (newUserPipe < 0 || dummyReadFd < 0) {
        cerr << "open() error! " << strerror(errno) << " " << errno << endl;
        return false;
    }


    return true;
}


bool PipeManager::loadUserPipe(int fromId, int toId) {
    string path = "./user_pipe/" + to_string(fromId) + "_" + to_string(toId);

    currentUserPipe = open(path.c_str(), O_RDONLY);

    if (currentUserPipe < 0) {
        return false;
    }

    unlink(path.c_str());

    return true;
}

bool PipeManager::openFromUserPipe(int fromId, int toId) {
    pair<int, int> key = pair<int, int>(fromId, toId);

    string path = "./user_pipe/" + to_string(fromId) + "_" + to_string(toId);

    int fromUserPipe = open(path.c_str(), O_WRONLY | O_NONBLOCK);
    userPipeMap[key] = fromUserPipe;

    return true;
}


bool PipeManager::closeFromUserPipe(int fromId, int toId) {
    pair<int, int> key = pair<int, int>(fromId, toId);
    auto entry = userPipeMap.find(key);
    if (entry == userPipeMap.end()) {
        return false;
    }

    int fromUserPipe = entry->second;
    close(fromUserPipe);

    userPipeMap.erase(key);

    return true;
}


bool PipeManager::closeUserPipe(int id) {
    for (const auto& entry : std::filesystem::directory_iterator(fifoPath)) {
        if (!entry.is_fifo()) {
            continue;
        }
        string filename = entry.path().filename();
        string id1 = filename.substr(0, filename.find("_"));
        string id2 = filename.substr(filename.find("_") + 1);
        if (id1 == to_string(id) || id2 == to_string(id)) {
            Message message;
            message.pid = UserManager::getUserById(id).pid;
            message.type = "closeFromUserPipe";
            message.value = id1 + "_" + id2;
            MessageManager::addMessage(message);

            unlink(entry.path().c_str());
        }
    }
    return true;
}

bool PipeManager::closeAllPipe() {
    for (const auto& entry : std::filesystem::directory_iterator(fifoPath)) {
        if (!entry.is_fifo()) {
            continue;
        }
        unlink(entry.path().c_str());
    }
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
