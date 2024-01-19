#pragma once
#include <linc/lexer/Token.hpp>
#include <linc/Include.hpp>

namespace linc
{
    struct NodeInfo
    {
        std::vector<Token> tokenList{};
        size_t lineNumber{0};
        std::string filename{};
    };
}