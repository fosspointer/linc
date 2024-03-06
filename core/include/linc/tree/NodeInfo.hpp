#pragma once
#include <linc/lexer/Token.hpp>
#include <linc/Include.hpp>

namespace linc
{
    struct NodeInfo
    {
        std::vector<Token> tokenList{};
        Token::Info info;
    };
}