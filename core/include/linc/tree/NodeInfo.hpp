#pragma once
#include <vector>
#include <string>
#include <linc/lexer/Token.hpp>

namespace linc
{
    struct NodeInfo
    {
        std::vector<Token> tokenList{};
        size_t lineNumber{0};
        std::string filename{};
    };
}