#ifndef _UTIL_H_
#define _UTIL_H_
#include "Util.h"
#endif


const char** Util::createArgv(const std::string& command, const std::vector<std::string>& args) {
    const char** argv = new const char*[args.size() + 2];
    argv[0] = command.c_str();
    for (int i = 0; i < int(args.size()); i++) {
        argv[i + 1] = args[i].c_str();
    }
    argv[args.size() + 1] = nullptr;
    return argv;
}