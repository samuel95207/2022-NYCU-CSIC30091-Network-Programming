#include <string>
#include <vector>
#include <utility>


struct NumberedPipe {
    int count;
    int pipe[2];
    bool includeStderr;
};

class PipeManager {
    const int READ = 0;
    const int WRITE = 1;

    std::vector<std::pair<int, int>> currentPipes;
    int newPipe[2];

    std::vector<NumberedPipe> numberedPipes;


   public:
    PipeManager();
    bool newSession();
    bool rootPipeHandler(bool pipeEnd = false, std::string outFilename = "");
    bool parentPipeHandler(bool pipeEnd = false, std::string outFilename = "");
    bool childPipeHandler(bool pipeEnd = false, std::string outFilename = "");

    bool addNumberedPipe(int count, bool includeStderr);
    void reduceNumberedPipesCount();
};