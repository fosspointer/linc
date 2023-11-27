#pragma once
#include <string>

namespace linc
{
    struct NodeInfo
    {
        std::vector<Token> tokenList{};
        bool isValid{false};
        size_t lineNumber{0};
        std::string filename{};
    };
}