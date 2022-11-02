#ifndef _SINGLE_PROC_SERVER_H_
#define _SINGLE_PROC_SERVER_H_
#include "SingleProcServer.h"
#endif

#ifndef _PASSIVESOCK_H_
#define _PASSIVESOCK_H_
#include "passivesock.h"
#endif

#include <arpa/inet.h>
#include <memory.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;


SingleProcServer::SingleProcServer(int port) : port(port) {}

void SingleProcServer::run() {
    sockaddr_in clientAddr;


    masterSocket = passivesock(to_string(port).c_str(), "TCP", QUEUE_LENGTH);
    if (masterSocket < 0) {
        return;
    }

    cout << "Listening on port " << port << endl;

    numFds = getdtablesize();
    FD_ZERO(&activeFds);
    FD_SET(masterSocket, &activeFds);


    while (true) {
        memcpy(&readFds, &activeFds, sizeof(readFds));

        if (select(numFds, &readFds, (fd_set *)0, (fd_set *)0, (timeval *)0) < 0) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "select: %s\n", strerror(errno));
            return;
        }




        if (FD_ISSET(masterSocket, &readFds)) {
            int clientAddrSize = sizeof(clientAddr);
            int slaveSocket = accept(masterSocket, (struct sockaddr *)&clientAddr, (socklen_t *)&clientAddrSize);

            if (slaveSocket < 0) {
                fprintf(stderr, "accept: %s\n", strerror(errno));
                return;
            }

            cout << "New user connected from " << inet_ntoa(clientAddr.sin_addr) << ":"
                 << (int)ntohs(clientAddr.sin_port) << endl;


            newClient(slaveSocket, clientAddr);
        }
        for (int fd = 0; fd < numFds; fd++) {
            if (fd == masterSocket || !FD_ISSET(fd, &readFds)) {
                continue;
            }



            int savedStdout = dup(fileno(stdout));
            int savedStderr = dup(fileno(stderr));
            dup2(fd, fileno(stdout));
            dup2(fd, fileno(stderr));


            char buf[BUF_SIZE];
            int readSize;
            NPShell *shell = npshellMap[fd];


            readSize = read(fd, buf, sizeof(buf));
            string inStr = string(buf).substr(0, readSize - 1);
            if (!inStr.empty() && inStr[inStr.size() - 1] == '\r') {
                inStr.erase(inStr.size() - 1);
            }

            // cout << "input = " << inStr << endl;
            // cout << "size = " << inStr.length() << endl;

            shell->execute(inStr, *this, fd);




            if (shell->getExit()) {
                dup2(savedStdout, fileno(stdout));
                dup2(savedStderr, fileno(stderr));
                close(savedStdout);
                close(savedStderr);

                delete shell;
                npshellMap.erase(fd);

                closeClient(fd);

                continue;
            }


            cout << NPShell::getSymbol();
            cout.flush();


            dup2(savedStdout, fileno(stdout));
            dup2(savedStderr, fileno(stderr));
            close(savedStdout);
            close(savedStderr);
        }
    }
}



void SingleProcServer::newClient(int fd, sockaddr_in ipAddr) {
    FD_SET(fd, &activeFds);

    npshellMap[fd] = new NPShell();
    BuildinCommand::execute(*npshellMap[fd], *this, fd, "setenv", {vector<string>({"PATH", "bin:."})});


    int savedStdout = dup(fileno(stdout));
    int savedStderr = dup(fileno(stderr));
    dup2(fd, fileno(stdout));
    dup2(fd, fileno(stderr));

    User *user = userManager.addUser(fd, ipAddr);

    cout << "****************************************" << endl;
    cout << "** Welcome to the information server. **" << endl;
    cout << "****************************************" << endl;

    string ipString = string(inet_ntoa(user->ipAddr.sin_addr)) + ":" + to_string((int)ntohs(user->ipAddr.sin_port));
    broadcast("*** User '" + (user->name == "" ? "(no name)" : user->name) + "' entered from " + ipString + ". ***\n");



    cout << NPShell::getSymbol();
    cout.flush();


    dup2(savedStdout, fileno(stdout));
    dup2(savedStderr, fileno(stderr));
    close(savedStdout);
    close(savedStderr);
}

void SingleProcServer::closeClient(int fd) {
    close(fd);
    FD_CLR(fd, &activeFds);

    User *user = userManager.getUserByFd(fd);
    broadcast("*** User '" + (user->name == "" ? "(no name)" : user->name) + "' left. ***\n");

    delete npshellMap[fd];
    npshellMap.erase(fd);

    cout << "User " << inet_ntoa(user->ipAddr.sin_addr) << ":" << (int)ntohs(user->ipAddr.sin_port) << " left." << endl;

    userManager.removeUserByFd(fd);

    close(fd);
}


void SingleProcServer::broadcast(string message) {
    for (int fd = 0; fd < numFds; fd++) {
        if (fd == masterSocket || !FD_ISSET(fd, &activeFds)) {
            continue;
        }

        int savedStdout = dup(fileno(stdout));
        int savedStderr = dup(fileno(stderr));
        dup2(fd, fileno(stdout));
        dup2(fd, fileno(stderr));


        cout << message;
        cout.flush();


        dup2(savedStdout, fileno(stdout));
        dup2(savedStderr, fileno(stderr));
        close(savedStdout);
        close(savedStderr);
    }
}

void SingleProcServer::sendDirectMessage(int id, std::string message) {
    User *user = userManager.getUserById(id);
    if (user == nullptr) {
        return;
    }

    int fd = user->fd;

    int savedStdout = dup(fileno(stdout));
    int savedStderr = dup(fileno(stderr));
    dup2(fd, fileno(stdout));
    dup2(fd, fileno(stderr));

    cout << message;

    dup2(savedStdout, fileno(stdout));
    dup2(savedStderr, fileno(stderr));
    close(savedStdout);
    close(savedStderr);
}
