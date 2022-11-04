#include <string>
#include <unordered_map>
#include <vector>

#ifndef _PIPE_MANAGER_H_
#define _PIPE_MANAGER_H_
#include "PipeManager.h"
#endif


class MultiProcServer;

class NPShell {
    inline static const std::string symbol = "% ";
    PipeManager pipeManager;
    bool exitFlag = false;

    std::unordered_map<std::string, std::pair<std::string, std::string>> envMap;

    std::string historyFilePath;


    friend class MultiProcServer;

   public:
    NPShell();
    void execute(std::string commandRaw, MultiProcServer &server, int fd);

    bool getExit();

    void setEnv(std::string env, std::string value);
    std::string getEnv(std::string env);
    void loadEnv();
    void resetEnv();

    static std::string getSymbol();
    friend class BuildinCommand;

   private:
    bool executeForkedCommand(const std::string &command, const std::vector<std::string> &args, PipeMode pipeMode,
                              PipeMode pipeMode2 = PipeMode::NONE, std::string outFilename = "");
    void setExit();
};
