#include <string>

class PipeManager {
    const int READ = 0;
    const int WRITE = 1;

    int normalPipe[2];
    int tmpPipe[2];


   public:
    PipeManager();
    bool newSession();
    bool rootPipeHandler(bool pipeEnd = false, std::string outFilename = "");
    bool parentPipeHandler(bool pipeEnd = false, std::string outFilename = "");
    bool childPipeHandler(bool pipeEnd = false, std::string outFilename = "");
};