#include <string>
#include <vector>

class NPShell {
    const std::string symbol = "% ";
    bool exitFlag = false;

   public:
    NPShell();
    void run();

    friend class BuildinCommand;

   private:
    void executeCommand(const std::string& command,const std::vector<std::string>& args);
    void setExit();
};