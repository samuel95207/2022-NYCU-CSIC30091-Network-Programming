#ifndef _MULTI_PROC_SERVER_H_
#define _MULTI_PROC_SERVER_H_
#include "MultiProcServer.h"
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


MultiProcServer::MultiProcServer(int port) : port(port) {
    signal(SIGCHLD, MultiProcServer::childSignalHandler);
    signal(SIGINT, MultiProcServer::intSignalHandler);
    userManager.setupSharedMemory();
}

void MultiProcServer::run() {
    sockaddr_in clientAddr;

    masterSocket = passivesock(to_string(port).c_str(), "TCP", QUEUE_LENGTH);

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
            // cerr << "accept: " << sys_errlist[errno] << " " << errno << endl;
            continue;
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

            pid = getpid();

            cout << "New user connected from " << inet_ntoa(clientAddr.sin_addr) << ":"
                 << (int)ntohs(clientAddr.sin_port) << endl;




            newClient(pid, slaveSocket, clientAddr);


            int savedStdout = dup(fileno(stdout));
            int savedStderr = dup(fileno(stderr));
            int savedStdin = dup(fileno(stdin));
            dup2(slaveSocket, fileno(stdout));
            dup2(slaveSocket, fileno(stderr));
            dup2(slaveSocket, fileno(stdin));


            string commandRaw;
            while (getline(cin, commandRaw)) {
                // cout << pid << " " << slaveSocket << endl;
                shell.execute(commandRaw, *this, pid, slaveSocket);
                if (shell.getExit()) {
                    break;
                }
                cout << NPShell::getSymbol();
            }



            dup2(savedStdout, fileno(stdout));
            dup2(savedStderr, fileno(stderr));
            dup2(savedStdin, fileno(stdin));
            close(savedStdout);
            close(savedStderr);
            close(savedStdin);

            closeClient(pid, slaveSocket);

            close(slaveSocket);
        }
    }


    // sockaddr_in clientAddr;


    // masterSocket = passivesock(to_string(port).c_str(), "TCP", QUEUE_LENGTH);
    // if (masterSocket < 0) {
    //     return;
    // }

    // cout << "Listening on port " << port << endl;

    // numFds = getdtablesize();
    // FD_ZERO(&activeFds);
    // FD_SET(masterSocket, &activeFds);


    // while (true) {
    //     memcpy(&readFds, &activeFds, sizeof(readFds));

    //     if (select(numFds, &readFds, (fd_set *)0, (fd_set *)0, (timeval *)0) < 0) {
    //         if (errno == EINTR) {
    //             continue;
    //         }
    //         fprintf(stderr, "select: %s\n", strerror(errno));
    //         return;
    //     }




    //     if (FD_ISSET(masterSocket, &readFds)) {
    //         int clientAddrSize = sizeof(clientAddr);
    //         int slaveSocket = accept(masterSocket, (struct sockaddr *)&clientAddr, (socklen_t *)&clientAddrSize);

    //         if (slaveSocket < 0) {
    //             fprintf(stderr, "accept: %s\n", strerror(errno));
    //             return;
    //         }

    //         cout << "New user connected from " << inet_ntoa(clientAddr.sin_addr) << ":"
    //              << (int)ntohs(clientAddr.sin_port) << endl;


    //         newClient(slaveSocket, clientAddr);
    //     }
    //     for (int fd = 0; fd < numFds; fd++) {
    //         if (fd == masterSocket || !FD_ISSET(fd, &readFds)) {
    //             continue;
    //         }



    //         int savedStdout = dup(fileno(stdout));
    //         int savedStderr = dup(fileno(stderr));
    //         dup2(fd, fileno(stdout));
    //         dup2(fd, fileno(stderr));


    //         char buf[BUF_SIZE];
    //         int readSize;
    //         NPShell *shell = npshellMap[fd];


    //         readSize = read(fd, buf, sizeof(buf));
    //         string inStr = string(buf).substr(0, readSize - 1);
    //         if (!inStr.empty() && inStr[inStr.size() - 1] == '\r') {
    //             inStr.erase(inStr.size() - 1);
    //         }

    //         // cout << "input = " << inStr << endl;
    //         // cout << "size = " << inStr.length() << endl;

    //         shell->execute(inStr, *this, fd);




    //         if (shell->getExit()) {
    //             dup2(savedStdout, fileno(stdout));
    //             dup2(savedStderr, fileno(stderr));
    //             close(savedStdout);
    //             close(savedStderr);

    //             delete shell;
    //             npshellMap.erase(fd);

    //             closeClient(fd);

    //             continue;
    //         }


    //         cout << NPShell::getSymbol();
    //         cout.flush();


    //         dup2(savedStdout, fileno(stdout));
    //         dup2(savedStderr, fileno(stderr));
    //         close(savedStdout);
    //         close(savedStderr);
    //     }
    // }
}



void MultiProcServer::newClient(int pid, int fd, sockaddr_in ipAddr) {
    BuildinCommand::execute(shell, *this, pid, fd, "setenv", {vector<string>({"PATH", "bin:."})});

    int savedStdout = dup(fileno(stdout));
    int savedStderr = dup(fileno(stderr));
    dup2(fd, fileno(stdout));
    dup2(fd, fileno(stderr));

    User *user = userManager.addUser(pid, fd, ipAddr);

    cout << "****************************************" << endl;
    cout << "** Welcome to the information server. **" << endl;
    cout << "****************************************" << endl;

    broadcast("*** User '" + (user->name == "" ? "(no name)" : user->name) + "' entered from " + user->ipAddr +
              ".***\n");



    cout << NPShell::getSymbol();
    cout.flush();


    dup2(savedStdout, fileno(stdout));
    dup2(savedStderr, fileno(stderr));
    close(savedStdout);
    close(savedStderr);
}

void MultiProcServer::closeClient(int pid, int fd) {
    User *user = userManager.getUserByPid(pid);

    broadcast("*** User '" + (user->name == "" ? "(no name)" : user->name) + "' left. ***\n");

    PipeManager::closeUserPipe(user->id);

    cout << "User " << user->ipAddr << " left." << endl;

    userManager.removeUserByPid(pid);
}


void MultiProcServer::broadcast(string message) {
    //     for (int fd = 0; fd < numFds; fd++) {
    //         if (fd == masterSocket || !FD_ISSET(fd, &activeFds)) {
    //             continue;
    //         }

    //         int savedStdout = dup(fileno(stdout));
    //         int savedStderr = dup(fileno(stderr));
    //         dup2(fd, fileno(stdout));
    //         dup2(fd, fileno(stderr));


    //         cout << message;
    //         cout.flush();


    //         dup2(savedStdout, fileno(stdout));
    //         dup2(savedStderr, fileno(stderr));
    //         close(savedStdout);
    //         close(savedStderr);
    //     }
}

void MultiProcServer::sendDirectMessage(int id, std::string message) {
    //     User *user = userManager.getUserById(id);
    //     if (user == nullptr) {
    //         return;
    //     }

    //     int fd = user->fd;

    //     int savedStdout = dup(fileno(stdout));
    //     int savedStderr = dup(fileno(stderr));
    //     dup2(fd, fileno(stdout));
    //     dup2(fd, fileno(stderr));

    //     cout << message;

    //     dup2(savedStdout, fileno(stdout));
    //     dup2(savedStderr, fileno(stderr));
    //     close(savedStdout);
    //     close(savedStderr);
}

void MultiProcServer::childSignalHandler(int signum) {
    int status;
    while (wait3(&status, WNOHANG, (rusage *)0) >= 0) {
    }
}

void MultiProcServer::intSignalHandler(int signum) {
    UserManager::closedSharedMemory();
    cout << "Shared Memory removed." << endl;
    exit(0);
}
