#include <string>
#include <vector>

#ifndef _PIPE_MANAGER_H_
#define _PIPE_MANAGER_H_
#include "PipeManager.h"
#endif

class NPShell {
    const std::string symbol = "% ";
    PipeManager pipeManager;
    bool exitFlag = false;

   public:
    NPShell();
    void run();

    friend class BuildinCommand;

   private:
    bool executeForkedCommand(const std::string &command, const std::vector<std::string> &args, PipeMode pipeMode,
                              std::string outFilename = "");
    void setExit();

    static void childSignalHandler(int signum);
};