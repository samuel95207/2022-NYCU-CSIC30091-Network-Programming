#include <string>
#include <unordered_map>
#include <vector>

#ifndef _NPSHELL_H_
#define _NPSHELL_H_
#include "NPShell.h"
#endif

typedef bool (*BuildinCommandFunction)(NPShell&, SingleProcServer&, int fd, const std::string&,
                                       const std::vector<std::string>&);

class BuildinCommand {
    static std::unordered_map<std::string, BuildinCommandFunction> commands;

   public:
    static bool isBuildinCommand(std::string command);
    static bool execute(NPShell& shell, SingleProcServer& server, int fd, const std::string& command,
                        const std::vector<std::string>& args);

   private:
    static bool exitCommand(NPShell& shell, SingleProcServer& server, int fd, const std::string& command,
                            const std::vector<std::string>& args);
    static bool printenvCommand(NPShell& shell, SingleProcServer& server, int fd, const std::string& command,
                                const std::vector<std::string>& args);
    static bool setenvCommand(NPShell& shell, SingleProcServer& server, int fd, const std::string& command,
                              const std::vector<std::string>& args);
    static bool whoCommand(NPShell& shell, SingleProcServer& server, int fd, const std::string& command,
                           const std::vector<std::string>& args);
    static bool yellCommand(NPShell& shell, SingleProcServer& server, int fd, const std::string& command,
                            const std::vector<std::string>& args);
    static bool nameCommand(NPShell& shell, SingleProcServer& server, int fd, const std::string& command,
                            const std::vector<std::string>& args);
};