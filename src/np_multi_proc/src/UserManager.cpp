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

#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include "UserManager.h"
#endif

#ifndef _SEM_H_
#define _SEM_H_
#include "sem.h"
#endif


using namespace std;


int UserManager::shmid;
char* UserManager::shmBuf;
int UserManager::sem;

UserManager::UserManager() {}


User UserManager::addUser(int pid, int fd, sockaddr_in ipAddr) {
    sem_wait(sem);
    readFromSharedMemory(false);

    string ipString = string(inet_ntoa(ipAddr.sin_addr)) + ":" + to_string((int)ntohs(ipAddr.sin_port));

    User newUser;
    newUser.pid = pid;
    newUser.name = "";
    newUser.fd = fd;
    newUser.ipAddr = ipString;
    newUser.id = idMin();


    idUserMap[newUser.id] = newUser;
    pidUserMap[pid] = newUser;

    writeToSharedMemory(false);
    sem_signal(sem);

    return newUser;
}

void UserManager::removeUserById(int id) {
    sem_wait(sem);
    readFromSharedMemory(false);

    auto result = idUserMap.find(id);
    if (result == idUserMap.end()) {
        sem_signal(sem);
        return;
    }
    idUserMap.erase(id);
    pidUserMap.erase(result->second.pid);
    if (result->second.name != "") {
        nameUserMap.erase(result->second.name);
    }

    writeToSharedMemory(false);
    sem_signal(sem);
}

void UserManager::removeUserByPid(int pid) {
    sem_wait(sem);
    readFromSharedMemory(false);

    auto result = pidUserMap.find(pid);
    if (result == pidUserMap.end()) {
        sem_signal(sem);
        return;
    }
    idUserMap.erase(result->second.id);
    pidUserMap.erase(pid);
    if (result->second.name != "") {
        nameUserMap.erase(result->second.name);
    }

    writeToSharedMemory(false);
    sem_signal(sem);
}

User UserManager::getUserById(int id, bool lock) {
    readFromSharedMemory(lock);


    auto result = idUserMap.find(id);
    if (result == idUserMap.end()) {
        User nullUser;
        nullUser.pid = -1;
        nullUser.id = -1;
        nullUser.fd = -1;
        return nullUser;
    }
    return result->second;
}

User UserManager::getUserByPid(int pid, bool lock) {
    readFromSharedMemory(lock);

    auto result = pidUserMap.find(pid);
    if (result == idUserMap.end()) {
        User nullUser;
        nullUser.pid = -1;
        nullUser.id = -1;
        nullUser.fd = -1;
        return nullUser;
    }
    return result->second;
}

User UserManager::getUserByName(string name, bool lock) {
    readFromSharedMemory(lock);

    if (name == "") {
        User nullUser;
        nullUser.pid = -1;
        nullUser.id = -1;
        nullUser.fd = -1;
        return nullUser;
    }
    auto result = nameUserMap.find(name);
    if (result == nameUserMap.end()) {
        User nullUser;
        nullUser.pid = -1;
        nullUser.id = -1;
        nullUser.fd = -1;
        return nullUser;
    }
    return result->second;
}


std::map<int, User> UserManager::getIdUserMap() { return idUserMap; }



bool UserManager::setNameById(int id, string name) {
    sem_wait(sem);

    readFromSharedMemory(false);

    User user = getUserByName(name, false);
    if (user.pid != -1 || name == "") {
        sem_signal(sem);
        return false;
    }

    user = idUserMap[id];

    nameUserMap.erase(user.name);
    user.name = name;

    nameUserMap[name] = user;
    idUserMap[user.id] = user;
    pidUserMap[user.pid] = user;


    writeToSharedMemory(false);
    sem_signal(sem);

    return true;
}


bool UserManager::setupSharedMemory() {
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

    // cout << "Setup shm" << endl;


    writeToSharedMemory();


    return true;
}

bool UserManager::readFromSharedMemory(bool lock) {
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



    idUserMap.clear();
    pidUserMap.clear();
    nameUserMap.clear();

    // cout << "Read from shm: " << endl;
    // cout << shmBuf;


    istringstream iss;
    iss.str(shmBuf);

    string line;
    while (getline(iss, line)) {
        istringstream lineIss;
        lineIss.str(line);

        int id, pid, fd;
        string ipAddr, name;

        lineIss >> id >> pid >> fd >> ipAddr;
        if (!(lineIss >> name)) {
            name = "";
        }

        // cout << "|" << id << "|" << pid << "|" << fd << "|" << ipAddr << "|" << name << "|" << endl;

        User newUser;
        newUser.id = id;
        newUser.pid = pid;
        newUser.fd = fd;
        newUser.ipAddr = ipAddr;
        newUser.name = name;


        idUserMap[newUser.id] = newUser;
        pidUserMap[pid] = newUser;

        if (name != "") {
            nameUserMap[name] = newUser;
        }
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

bool UserManager::writeToSharedMemory(bool lock) {
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
    for (auto idUserPair : idUserMap) {
        User user = idUserPair.second;
        oss << user.id << " " << user.pid << " " << user.fd << " " << user.ipAddr << " " << user.name << endl;
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



int UserManager::idMin() {
    int id = 1;
    while (true) {
        if (idUserMap.find(id) == idUserMap.end()) {
            return id;
        }
        id++;
    }
}

void UserManager::closedSharedMemory() {
    shmctl(shmid, IPC_RMID, (struct shmid_ds*)0);
    sem_close(sem);
}