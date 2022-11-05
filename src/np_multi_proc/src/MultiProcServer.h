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


#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include "UserManager.h"
#endif

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif

class BuildinCommand;
class MessageManager;

class MultiProcServer {
    const int QUEUE_LENGTH = 1024;

    int port;
    int masterSocket;

    NPShell shell;


    UserManager userManager;
    MessageManager messageManager;


    friend class BuildinCommand;
    friend class NPShell;
    friend class MessageManager;


   public:
    MultiProcServer(int port);
    void run();

    static void childSignalHandler(int signum);
    static void intSignalHandler(int signum);



   private:
    void newClient(int pid, int fd, sockaddr_in ipAddr);
    void closeClient(int pid, int fd);

    void broadcast(std::string message, bool includeSelf = true);
    void sendDirectMessage(int id, std::string message);
};