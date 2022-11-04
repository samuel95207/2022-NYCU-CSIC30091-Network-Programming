
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>

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


User* UserManager::addUser(int pid, int fd, sockaddr_in ipAddr) {
    sem_wait(sem);
    readFromSharedMemory();

    string ipString = string(inet_ntoa(ipAddr.sin_addr)) + ":" + to_string((int)ntohs(ipAddr.sin_port));

    User* newUser = new User();
    newUser->pid = pid;
    newUser->name = "";
    newUser->fd = fd;
    newUser->ipAddr = ipString;
    newUser->id = idMin();


    idUserMap[newUser->id] = newUser;
    pidUserMap[pid] = newUser;

    writeToSharedMemory();
    sem_signal(sem);

    return newUser;
}

void UserManager::removeUserById(int id) {
    sem_wait(sem);
    readFromSharedMemory();

    auto result = idUserMap.find(id);
    if (result == idUserMap.end()) {
        return;
    }
    idUserMap.erase(id);
    pidUserMap.erase(result->second->pid);
    if (result->second->name != "") {
        nameUserMap.erase(result->second->name);
    }
    delete result->second;

    writeToSharedMemory();
    sem_signal(sem);
}

void UserManager::removeUserByPid(int pid) {
    sem_wait(sem);
    readFromSharedMemory();

    auto result = pidUserMap.find(pid);
    if (result == pidUserMap.end()) {
        return;
    }
    idUserMap.erase(result->second->id);
    pidUserMap.erase(pid);
    if (result->second->name != "") {
        nameUserMap.erase(result->second->name);
    }
    delete result->second;

    writeToSharedMemory();
    sem_signal(sem);
}

User* UserManager::getUserById(int id) {
    sem_wait(sem);
    readFromSharedMemory();
    sem_signal(sem);

    auto result = idUserMap.find(id);
    if (result == idUserMap.end()) {
        return nullptr;
    }
    return result->second;
}

User* UserManager::getUserByPid(int pid) {
    sem_wait(sem);
    readFromSharedMemory();
    sem_signal(sem);


    auto result = pidUserMap.find(pid);
    if (result == idUserMap.end()) {
        return nullptr;
    }
    return result->second;
}

User* UserManager::getUserByName(string name) {
    if (name == "") {
        return nullptr;
    }
    auto result = nameUserMap.find(name);
    if (result == nameUserMap.end()) {
        return nullptr;
    }
    return result->second;
}

map<int, User*> UserManager::getIdUserMap() {
    return idUserMap;
}
map<int, User*> UserManager::getPidUserMap() {
    return pidUserMap;
}
map<string, User*> UserManager::getNameUserMap() {
    return nameUserMap;
}


bool UserManager::setNameById(int id, string name) {
    sem_wait(sem);
    readFromSharedMemory();

    User* user = getUserByName(name);
    if (user != nullptr || name == "") {
        return false;
    }

    user = idUserMap[id];

    nameUserMap.erase(user->name);
    user->name = name;
    nameUserMap[name] = user;


    writeToSharedMemory();
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

    cout << "Setup shm" << endl;

    sem_wait(sem);
    writeToSharedMemory();
    sem_signal(sem);


    return true;
}

bool UserManager::readFromSharedMemory() {
    shmBuf = (char*)shmat(shmid, (char*)0, 0);
    if (shmBuf == (void*)-1) {
        cerr << "shmat() error!" << endl;
        return false;
    }



    for (auto userPair : idUserMap) {
        if (userPair.second != nullptr) {
            delete userPair.second;
        }
    }


    idUserMap.clear();
    pidUserMap.clear();
    nameUserMap.clear();

    cout << "Read from shm: " << endl;
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

        cout << "|" << id << "|" << pid << "|" << fd << "|" << ipAddr << "|" << name << "|" << endl;

        User* newUser = new User();
        newUser->id = id;
        newUser->pid = pid;
        newUser->fd = fd;
        newUser->ipAddr = ipAddr;
        newUser->name = name;


        idUserMap[newUser->id] = newUser;
        pidUserMap[pid] = newUser;

        if (name != "") {
            nameUserMap[name] = newUser;
        }
    }


    if (shmdt(shmBuf) < 0) {
        cerr << "shdt() error!" << endl;
        return false;
    }

    return true;
}

bool UserManager::writeToSharedMemory() {
    shmBuf = (char*)shmat(shmid, (char*)0, 0);
    if (shmBuf == (void*)-1) {
        cerr << "shmat() error!" << endl;
        return false;
    }


    ostringstream oss;
    for (auto idUserPair : idUserMap) {
        User* user = idUserPair.second;
        oss << user->id << " " << user->pid << " " << user->fd << " " << user->ipAddr << " " << user->name << endl;
    }
    cout << "Write to shm: " << endl;
    cout << oss.str();
    strcpy(shmBuf, oss.str().c_str());


    if (shmdt(shmBuf) < 0) {
        cerr << "shdt() error!" << endl;
        return false;
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