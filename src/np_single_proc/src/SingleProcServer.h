#include <string>
#include <unordered_map>

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif


class SingleProcServer {
    const int QUEUE_LENGTH = 5;
    const int BUF_SIZE = 4098;

    int port;

    fd_set readFds;
    fd_set activeFds;
    std::unordered_map<int, NPShell*> npshellMap;

   public:
    SingleProcServer(int port);
    void run();


   private:
    void newClient(int fd);
    void closeClient(int fd);
};