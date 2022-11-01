#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include "UserManager.h"
#endif

using namespace std;

UserManager::UserManager() {}


User* UserManager::addUser(int fd, sockaddr_in ipAddr) {
    User* newUser = new User();
    newUser->name = "";
    newUser->fd = fd;
    newUser->ipAddr = ipAddr;
    newUser->id = idMin();

    string ipString = string(inet_ntoa(ipAddr.sin_addr)) + ":" + to_string((int)ntohs(ipAddr.sin_port));
    // cout << ipString << endl;
    if (ipUsernameMap.find(ipString) != ipUsernameMap.end()) {
        newUser->name = ipUsernameMap[ipString];
    }
    ipUsernameMap[ipString] = newUser->name;


    idUserMap[newUser->id] = newUser;
    fdUserMap[fd] = newUser;

    return newUser;
}

void UserManager::removeUserById(int id) {
    auto result = idUserMap.find(id);
    if (result == idUserMap.end()) {
        return;
    }
    idUserMap.erase(id);
    fdUserMap.erase(result->second->fd);
    delete result->second;
}

void UserManager::removeUserByFd(int fd) {
    auto result = idUserMap.find(fd);
    if (result == idUserMap.end()) {
        return;
    }
    idUserMap.erase(result->second->id);
    fdUserMap.erase(fd);
    delete result->second;
}

User* UserManager::getUserById(int id) {
    auto result = idUserMap.find(id);
    if (result == idUserMap.end()) {
        return nullptr;
    }
    return result->second;
}

User* UserManager::getUserByFd(int fd) {
    auto result = fdUserMap.find(fd);
    if (result == idUserMap.end()) {
        return nullptr;
    }
    return result->second;
}

map<int, User*> UserManager::getIdUserMap() { return idUserMap; }
map<int, User*> UserManager::getFdUserMap() { return fdUserMap; }




int UserManager::idMin() {
    int id = 1;
    while (true) {
        if (idUserMap.find(id) == idUserMap.end()) {
            return id;
        }
        id++;
    }
}