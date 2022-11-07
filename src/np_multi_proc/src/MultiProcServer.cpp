#ifndef _MULTI_PROC_SERVER_H_
#define _MULTI_PROC_SERVER_H_
#include "MultiProcServer.h"
#endif

#ifndef _PASSIVESOCK_H_
#define _PASSIVESOCK_H_
#include "passivesock.h"
#endif

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif

#ifndef _BUILDIN_COMMAND_H_
#define _BUILDIN_COMMAND_H_
#include "BuildinCommand.h"
#endif

#include <arpa/inet.h>
#include <memory.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;


MultiProcServer::MultiProcServer(int port) : port(port) {
    signal(SIGCHLD, MultiProcServer::childSignalHandler);
    signal(SIGINT, MultiProcServer::intSignalHandler);
    userManager.setupSharedMemory();
    messageManager.setupSharedMemory();
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
            cerr << "accept: " << strerror(errno) << " " << errno << endl;
            return;
        }

        pid_t pid;

        do {
            pid = fork();
        } while (pid == -1);

        if (pid > 0) {
            // Parent Process
            close(slaveSocket);
        } else {
            // Child Process
            close(masterSocket);

            pid = getpid();

            messageManager.setPidFd(pid, slaveSocket);

            pid_t messageManagerPid;

            do {
                messageManagerPid = fork();
            } while (messageManagerPid == -1);

            if (messageManagerPid > 0) {
                cout << "New user connected from " << inet_ntoa(clientAddr.sin_addr) << ":"
                     << (int)ntohs(clientAddr.sin_port) << " " << pid << endl;

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
                    if (!commandRaw.empty() && commandRaw[commandRaw.length() - 1] == '\r') {
                        commandRaw.erase(commandRaw.size() - 1);
                    }
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

                exit(0);
            } else {
                messageManager.run(*this);
                exit(0);
            }



            exit(0);
        }
    }
}



void MultiProcServer::newClient(int pid, int fd, sockaddr_in ipAddr) {
    BuildinCommand::execute(shell, *this, pid, fd, "setenv", {vector<string>({"PATH", "bin:."})});

    int savedStdout = dup(fileno(stdout));
    int savedStderr = dup(fileno(stderr));
    dup2(fd, fileno(stdout));
    dup2(fd, fileno(stderr));

    User user = userManager.addUser(pid, fd, ipAddr);

    string broadcastMessage =
        "*** User '" + (user.name == "" ? "(no name)" : user.name) + "' entered from " + user.ipAddr + ". ***";

    cout << "****************************************" << endl;
    cout << "** Welcome to the information server. **" << endl;
    cout << "****************************************" << endl;
    cout << broadcastMessage << endl;

    broadcast(broadcastMessage, false);



    cout << NPShell::getSymbol();
    cout.flush();


    dup2(savedStdout, fileno(stdout));
    dup2(savedStderr, fileno(stderr));
    close(savedStdout);
    close(savedStderr);
}

void MultiProcServer::closeClient(int pid, int fd) {
    User user = userManager.getUserByPid(pid);

    broadcast("*** User '" + (user.name == "" ? "(no name)" : user.name) + "' left. ***", false);

    PipeManager::closeUserPipe(user.id);

    cout << "User " << user.ipAddr << " left." << endl;


    Message message;
    message.pid = pid;
    message.type = "exit";
    message.value = "";
    messageManager.addMessage(message);

    userManager.removeUserByPid(pid);
}


void MultiProcServer::broadcast(string messageStr, bool includeSelf) {
    userManager.readFromSharedMemory();

    int selfPid = getpid();
    for (auto userPair : userManager.getIdUserMap()) {
        User user = userPair.second;

        if (!includeSelf && user.pid == selfPid) {
            continue;
        }

        Message message;
        message.pid = user.pid;
        message.type = "tell";
        message.value = messageStr;
        messageManager.addMessage(message);
    }
}

void MultiProcServer::sendDirectMessage(int id, std::string messageStr) {
    User user = userManager.getUserById(id);
    if (user.pid == -1) {
        return;
    }

    Message message;
    message.pid = user.pid;
    message.type = "tell";
    message.value = messageStr;
    messageManager.addMessage(message);
}

void MultiProcServer::childSignalHandler(int signum) {
    int status;
    wait3(&status, WNOHANG, (rusage *)0);
}

void MultiProcServer::intSignalHandler(int signum) {
    UserManager::closedSharedMemory();
    MessageManager::closedSharedMemory();
    PipeManager::closeAllPipe();
    cout << "Shared Memory removed." << endl;
    exit(0);
}
