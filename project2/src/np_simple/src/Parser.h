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
    static bool isOperator(std::string token);

   public:
    static ParseResult parse(std::string commandStr);
    static void printParseResult(const ParseResult& parseResult);
};