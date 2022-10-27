#include <string>
#include <vector>


class Util {
   public:
    static const char** createArgv(const std::string& command, const std::vector<std::string>& args);
};