#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum class PipeMode
{
	NORMAL_PIPE,
    FILE_OUTPUT,
	CONSOLE_OUTPUT,
	NUMBERED_PIPE,
    NUMBERED_PIPE_STDERR
};


class PipeManager {
    const int READ = 0;
    const int WRITE = 1;

    int currentPipe[2];
    int newPipe[2];
    int newNumberedPipe[2];

    int count = 0;
    std::unordered_map<int, std::pair<int, int>> countPipeMap;

   public:
    PipeManager();
    bool newSession();
    bool rootPipeHandler(PipeMode pipeMode, std::string outFilename = "");
    bool parentPipeHandler(PipeMode pipeMode, std::string outFilename = "");
    bool childPipeHandler(PipeMode pipeMode, std::string outFilename = "");

    void addNumberedPipe(int countIn);
    std::pair<int, int> getCurrentNumberedPipe();
    void reduceNumberedPipesCount();
};