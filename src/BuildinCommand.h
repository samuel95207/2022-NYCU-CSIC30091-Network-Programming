#include <string>
#include <unordered_map>
#include <vector>

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif

typedef bool (*BuildinCommandFunction)(NPShell&, const std::string&, const std::vector<std::string>&);

class BuildinCommand {
    static std::unordered_map<std::string, BuildinCommandFunction> commands;

   public:
    static bool isBuildinCommand(std::string command);
    static bool execute(NPShell& shell, const std::string& command, const std::vector<std::string>& args);

   private:
    static bool exitCommand(NPShell& shell, const std::string& command, const std::vector<std::string>& args);
    static bool printenvCommand(NPShell& shell, const std::string& command, const std::vector<std::string>& args);
    static bool setenvCommand(NPShell& shell, const std::string& command, const std::vector<std::string>& args);

};