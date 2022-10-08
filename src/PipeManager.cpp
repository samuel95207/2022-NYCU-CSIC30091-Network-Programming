#ifndef _PIPE_MANAGER_H_
#define _PIPE_MANAGER_H_
#include "PipeManager.h"
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

using namespace std;

PipeManager::PipeManager() {
    normalPipe[READ] = 0;
    normalPipe[WRITE] = 0;
}

bool PipeManager::newSession() {
    normalPipe[READ] = 0;
    normalPipe[WRITE] = 0;

    return true;
}


bool PipeManager::rootPipeHandler(bool pipeEnd, std::string outFilename) {
    if (!pipeEnd) {
        if (pipe(tmpPipe)) {
            return false;
        }
    }
    return true;
}

bool PipeManager::parentPipeHandler(bool pipeEnd, std::string outFilename) {
    if (normalPipe[READ] != 0 && normalPipe[WRITE] != 0) {
        close(normalPipe[READ]);
        close(normalPipe[WRITE]);
    }

    if (!pipeEnd) {
        normalPipe[READ] = tmpPipe[READ];
        normalPipe[WRITE] = tmpPipe[WRITE];
    }

    return true;
}

bool PipeManager::childPipeHandler(bool pipeEnd, std::string outFilename) {
    if (normalPipe[0] != 0 && normalPipe[1] != 0) {
        dup2(normalPipe[READ], fileno(stdin));
        close(normalPipe[READ]);

        close(normalPipe[WRITE]);
    }

    if (!pipeEnd) {
        close(tmpPipe[READ]);

        dup2(tmpPipe[WRITE], fileno(stdout));
        close(tmpPipe[WRITE]);
    } else {
        if (outFilename != "") {
            int permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
            int outfile = open(outFilename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, permission);
            dup2(outfile, fileno(stdout));
        }
    }

    return true;
}
