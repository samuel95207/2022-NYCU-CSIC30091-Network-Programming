#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum class PipeMode {
    NONE,
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

    const int FIFO_PERMS = 0666;
    static const std::string fifoPath;

    int currentPipe[2];
    int newPipe[2];

    int currentNumberedPipe[2];
    int newNumberedPipe[2];

    int dummyReadFd;
    int currentUserPipe;
    int newUserPipe;

    int count = 0;
    std::unordered_map<int, std::pair<int, int>> countPipeMap;

    std::map<std::pair<int, int>, int> userPipeMap;


   public:
    PipeManager();
    bool newSession();
    bool rootPipeHandler(PipeMode pipeMode, PipeMode pipeMode2 = PipeMode::NONE, std::string outFilename = "");
    bool parentPipeHandler(PipeMode pipeMode, PipeMode pipeMode2 = PipeMode::NONE, std::string outFilename = "");
    bool childPipeHandler(PipeMode pipeMode, PipeMode pipeMode2 = PipeMode::NONE, std::string outFilename = "");

    bool addNumberedPipe(int countIn);

    bool addUserPipe(int fromId, int toId);
    bool loadUserPipe(int fromId, int toId);
    bool openFromUserPipe(int fromId, int toId);
    bool closeFromUserPipe(int fromId, int toId);

    static bool closeUserPipe(int id);
    static bool closeAllPipe();



    void printCountPipeMap();

   private:
    void loadCurrentNumberedPipe();
    void reduceNumberedPipesCount();
};