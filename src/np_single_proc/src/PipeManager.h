#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum class PipeMode {
    NORMAL_PIPE,
    FILE_OUTPUT,
    CONSOLE_OUTPUT,
    NUMBERED_PIPE,
    NUMBERED_PIPE_STDERR,
    USER_PIPE_OUT,
    USER_PIPE_IN,
    USER_PIPE_BOTH
};


class PipeManager {
    const int READ = 0;
    const int WRITE = 1;

    int currentPipe[2];
    int newPipe[2];

    int currentNumberedPipe[2];
    int newNumberedPipe[2];

    int currentUserPipe[2];
    int newUserPipe[2];

    int count = 0;
    std::unordered_map<int, std::pair<int, int>> countPipeMap;

    static std::map<std::pair<int, int>, std::pair<int, int>> userPipeMap;


   public:
    PipeManager();
    bool newSession();
    bool rootPipeHandler(PipeMode pipeMode, std::string outFilename = "");
    bool parentPipeHandler(PipeMode pipeMode, std::string outFilename = "");
    bool childPipeHandler(PipeMode pipeMode, std::string outFilename = "");

    bool addNumberedPipe(int countIn);
    bool addUserPipe(int fromId, int toId);
    bool loadUserPipe(int fromId, int toId);


    void printCountPipeMap();

   private:
    void loadCurrentNumberedPipe();
    void reduceNumberedPipesCount();
};