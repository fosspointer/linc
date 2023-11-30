#pragma once
#include <optional>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <linc/lexer/Token.hpp>

namespace linc
{
    class Brackets final
    {
    public:
        using BracketMap = std::unordered_map<char, Token::Type>; 
        static Token::Type getBracket(char character);
    private:
        static const BracketMap s_BracketMap;
    };
}