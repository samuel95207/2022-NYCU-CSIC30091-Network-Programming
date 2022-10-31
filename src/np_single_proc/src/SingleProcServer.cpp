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


    int masterSocket = passivesock(to_string(port).c_str(), "TCP", QUEUE_LENGTH);
    if (masterSocket < 0) {
        return;
    }
    cout << "Listening on port " << port << endl;

    int numFds = getdtablesize();
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


            newClient(slaveSocket);
        }
        for (int fd = 0; fd < numFds; fd++) {
            if (fd == masterSocket || !FD_ISSET(fd, &readFds)) {
                continue;
            }



            if (npshellMap.find(fd) == npshellMap.end()) {
                npshellMap[fd] = new NPShell();
            }


            int savedStdout = dup(fileno(stdout));
            int savedStderr = dup(fileno(stderr));
            dup2(fd, fileno(stdout));
            dup2(fd, fileno(stderr));


            char buf[BUF_SIZE];
            int readSize;
            NPShell *shell = npshellMap[fd];


            readSize = read(fd, buf, sizeof(buf));
            string inStr = string(buf).substr(0, readSize - 2);

            // cout << "input = " << inStr << endl;
            // cout << "size = " << inStr.length() << endl;

            shell->execute(inStr);




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


        // int clientAddrSize = sizeof(clientAddr);
        // int slaveSocket = accept(masterSocket, (sockaddr *)&clientAddr, (socklen_t *)&clientAddrSize);

        // if (slaveSocket < 0) {
        //     if (errno == EINTR) {
        //         continue;
        //     }
        //     // cerr << "accept: " << sys_errlist[errno] << endl;
        //     return;
        // }

        // pid_t pid = fork();
        // if (pid == -1) {
        //     cerr << "Fork error!" << endl;
        //     return;
        // } else if (pid > 0) {
        //     // Parent Process
        //     close(slaveSocket);
        // } else {
        //     // Child Process
        //     close(masterSocket);

        //     cout << "New user connected from " << inet_ntoa(clientAddr.sin_addr) << ":"
        //          << (int)ntohs(clientAddr.sin_port) << endl;


        //     dup2(slaveSocket, fileno(stdin));
        //     dup2(slaveSocket, fileno(stdout));
        //     dup2(slaveSocket, fileno(stderr));

        //     NPShell shell = NPShell();
        //     shell.run();

        //     close(slaveSocket);
        // }
    }
}



void SingleProcServer::newClient(int fd) {
    FD_SET(fd, &activeFds);

    int savedStdout = dup(fileno(stdout));
    int savedStderr = dup(fileno(stderr));
    dup2(fd, fileno(stdout));
    dup2(fd, fileno(stderr));


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
}