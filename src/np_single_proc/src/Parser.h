#include <array>
#include <string>
#include <utility>
#include <vector>



struct ParseResult {
    std::vector<std::pair<std::string, std::vector<std::string>>> commands;
    std::vector<std::string> operators;
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