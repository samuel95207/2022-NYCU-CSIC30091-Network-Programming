#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>


#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#include "Console.h"
#endif

using namespace std;

int main(int argc, char* argv[]) {
    Console console;
    console.start();

    return 0;
}