#include <string>

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif


class SimpleServer{
    const int QUEUE_LENGTH = 1024;

    int port;

    public:
    SimpleServer(int port);
    void run();

    static void childSignalHandler(int signum);

};