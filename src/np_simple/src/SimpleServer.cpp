#ifndef _SIMPLE_SERVER_H_
#define _SIMPLE_SERVER_H_
#include "SimpleServer.h"
#endif

#ifndef _PASSIVESOCK_H_
#define _PASSIVESOCK_H_
#include "passivesock.h"
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <string>

using namespace std;


SimpleServer::SimpleServer(int port) : port(port) { signal(SIGCHLD, SimpleServer::childSignalHandler); }

void SimpleServer::run() {
    sockaddr_in clientAddr;

    int masterSocket = passivesock(to_string(port).c_str(), "TCP", QUEUE_LENGTH);

    if (masterSocket < 0) {
        return;
    }

    cout << "Listening on port " << port << endl;

    while (true) {
        int clientAddrSize = sizeof(clientAddr);
        int slaveSocket = accept(masterSocket, (sockaddr *)&clientAddr, (socklen_t *)&clientAddrSize);

        if (slaveSocket < 0) {
            if (errno == EINTR) {
                continue;
            }
            // cerr << "accept: " << sys_errlist[errno] << endl;
            return;
        }

        pid_t pid = fork();
        if (pid == -1) {
            cerr << "Fork error!" << endl;
            return;
        } else if (pid > 0) {
            // Parent Process
            close(slaveSocket);
        } else {
            // Child Process
            close(masterSocket);

            cout << "New user connected from " << inet_ntoa(clientAddr.sin_addr) << ":"
                 << (int)ntohs(clientAddr.sin_port) << endl;


            dup2(slaveSocket, fileno(stdin));
            dup2(slaveSocket, fileno(stdout));
            dup2(slaveSocket, fileno(stderr));

            NPShell shell = NPShell();
            shell.run();

            close(slaveSocket);
        }
    }
}


void SimpleServer::childSignalHandler(int signum) {
    int status;
    while (wait3(&status, WNOHANG, (rusage *)0) >= 0) {
    }
}
