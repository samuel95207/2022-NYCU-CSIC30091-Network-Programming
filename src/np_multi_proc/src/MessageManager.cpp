#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#ifndef _SEM_H_
#define _SEM_H_
#include "sem.h"
#endif

#ifndef _MESSAGE_MANAGER_H_
#define _MESSAGE_MANAGER_H_
#include "MessageManager.h"
#endif

using namespace std;

int MessageManager::shmid;
char* MessageManager::shmBuf;
int MessageManager::sem;

MessageManager::MessageManager() {}


void MessageManager::run() {
    while (true) {
        sem_wait(sem);

        readFromSharedMemory(false);

        vector<int> removeIdx;

        int idx = 0;
        bool exitFlag = false;
        for (auto message : messageQueue) {
            if (message.pid != pid) {
                continue;
            }

            removeIdx.push_back(idx);

            if (message.type == "tell") {
                int savedStdout = dup(fileno(stdout));
                int savedStderr = dup(fileno(stderr));
                dup2(fd, fileno(stdout));
                dup2(fd, fileno(stderr));


                cout << message.value << endl;


                dup2(savedStdout, fileno(stdout));
                dup2(savedStderr, fileno(stderr));
                close(savedStdout);
                close(savedStderr);

            } else if (message.type == "exit") {
                exitFlag = true;
                close(fd);
                break;
            }

            idx++;
        }

        for (int i = removeIdx.size() - 1; i >= 0; i--) {
            messageQueue.erase(messageQueue.begin() + removeIdx[i]);
        }

        writeToSharedMemory(false);

        sem_signal(sem);

        if (exitFlag) {
            break;
        }
    }

    close(fd);
}

void MessageManager::addMessage(const Message& message) {
    sem_wait(sem);

    readFromSharedMemory(false);
    messageQueue.push_back(message);
    writeToSharedMemory(false);

    sem_signal(sem);
}


void MessageManager::setPidFd(int pid_in, int fd_in) {
    pid = pid_in;
    fd = fd_in;
}


bool MessageManager::setupSharedMemory() {
    shmid = shmget(SHM_KEY, SHM_SIZE, SHM_PERMS | IPC_CREAT);
    if (shmid < 0) {
        cerr << "shmget() error! " << strerror(errno) << " " << errno << endl;
        return false;
    }

    sem = sem_create(SEM_KEY, 0);
    if (sem < 0) {
        cerr << "sem() error!" << endl;
        return false;
    }

    sem_signal(sem);

    cout << "Setup shm" << endl;

    writeToSharedMemory();

    return true;
}

void MessageManager::closedSharedMemory() {
    shmctl(shmid, IPC_RMID, (struct shmid_ds*)0);
    sem_close(sem);
}


bool MessageManager::readFromSharedMemory(bool lock) {
    if (lock) {
        sem_wait(sem);
    }
    shmBuf = (char*)shmat(shmid, (char*)0, 0);
    if (shmBuf == (void*)-1) {
        cerr << "shmat() error!" << endl;
        if (lock) {
            sem_signal(sem);
        }
        return false;
    }


    messageQueue.clear();

    // cout << shmBuf;


    istringstream iss;
    iss.str(shmBuf);

    string line;
    while (getline(iss, line)) {
        // cout << "Read from shm: " << endl;
        // cout << line << endl;
        istringstream lineIss;
        lineIss.str(line);

        int pid;
        string type, value;

        lineIss >> pid >> type >> ws;
        getline(lineIss, value);

        // cout << "|" << id << "|" << pid << "|" << fd << "|" << ipAddr << "|" << name << "|" << endl;

        Message message;
        message.pid = pid;
        message.type = type;
        message.value = value;

        messageQueue.push_back(message);
    }


    if (shmdt(shmBuf) < 0) {
        cerr << "shdt() error!" << endl;
        if (lock) {
            sem_signal(sem);
        }
        return false;
    }

    if (lock) {
        sem_signal(sem);
    }
    return true;
}

bool MessageManager::writeToSharedMemory(bool lock) {
    if (lock) {
        sem_wait(sem);
    }

    shmBuf = (char*)shmat(shmid, (char*)0, 0);
    if (shmBuf == (void*)-1) {
        cerr << "shmat() error!" << endl;
        if (lock) {
            sem_signal(sem);
        }
        return false;
    }


    ostringstream oss;
    for (auto message : messageQueue) {
        oss << message.pid << " " << message.type << " " << message.value << endl;
    }
    // cout << "Write to shm: " << endl;
    // cout << oss.str();

    strcpy(shmBuf, oss.str().c_str());


    if (shmdt(shmBuf) < 0) {
        cerr << "shdt() error!" << endl;
        if (lock) {
            sem_signal(sem);
        }
        return false;
    }

    if (lock) {
        sem_signal(sem);
    }
    return true;
}
