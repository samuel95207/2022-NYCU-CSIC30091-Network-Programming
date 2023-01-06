#include <array>
#include <string>
#include <utility>
#include <vector>


struct Command{
    std::string command;
    std::vector<std::string> args;
    std::vector<std::string> operators;
};

struct ParseResult {
    std::vector<Command> commands;
    std::string errorMessage;
};

class Parser {
    static const std::vector<std::string> operatorTypes;
    static const std::vector<std::string> oneArgCommands;
    static const std::vector<std::string> twoArgCommands;
    static bool isOperator(std::string token);
    static bool isOneArgCommand(std::string command);
    static bool isTwoArgCommand(std::string command);

   public:
    static ParseResult parse(std::string commandStr);
    static void printParseResult(const ParseResult& parseResult);
};