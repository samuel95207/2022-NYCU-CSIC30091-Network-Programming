#include <iostream>

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif

#ifndef _SIMPLE_SERVER_H_
#define _SIMPLE_SERVER_H_
#include "SimpleServer.h"
#endif

using namespace std;

int main(int argc, char** argv) {
    if(argc != 2){
        cerr << "Error! Please use input format \"./np_simple <port>\"." << endl;
        return -1;
    }

    int port = atoi(argv[1]);

    SimpleServer simpleServer(port);
    simpleServer.run();
    
}