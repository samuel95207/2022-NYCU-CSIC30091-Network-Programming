#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct NumberedPipe {
    int pipe[2];
    bool includeStderr;
};

class PipeManager {
    const int READ = 0;
    const int WRITE = 1;

    int currentPipe[2];
    int newPipe[2];

    int count = 0;
    std::unordered_map<int, std::pair<int, int>> countPipeMap;

   public:
    PipeManager();
    bool newSession();
    bool rootPipeHandler(NumberedPipe *numberedPipe = nullptr, bool pipeEnd = false, std::string outFilename = "");
    bool parentPipeHandler(NumberedPipe *numberedPipe = nullptr, bool pipeEnd = false, std::string outFilename = "");
    bool childPipeHandler(NumberedPipe *numberedPipe = nullptr, bool pipeEnd = false, std::string outFilename = "");

    NumberedPipe addNumberedPipe(int countIn, bool includeStderr);
    int *getCurrentNumberedPipe();
    void reduceNumberedPipesCount();
};