#include <arpa/inet.h>
#include <memory.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <map>
#include <string>
#include <unordered_map>

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif

#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include "UserManager.h"
#endif

#ifndef _BUILDIN_COMMAND_H_
#define _BUILDIN_COMMAND_H_
#include "BuildinCommand.h"
#endif

class BuildinCommand;

class MultiProcServer {
    const int QUEUE_LENGTH = 1024;

    int port;
    int masterSocket;


    UserManager userManager;


    friend class BuildinCommand;
    friend class NPShell;


   public:
    MultiProcServer(int port);
    void run();

    static void childSignalHandler(int signum);


   private:
    void newClient(int fd, sockaddr_in ipAddr);
    void closeClient(int fd);

    void broadcast(std::string message);
    void sendDirectMessage(int id, std::string message);
};