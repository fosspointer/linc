#pragma once
#include <linc/lexer/Token.hpp>
#include <linc/Include.hpp>

namespace linc
{
    /// @brief Lexer-token representation of bracket-pairs. 
    class Brackets final
    {
    public:
        using BracketMap = std::unordered_map<char, Token::Type>; 
        static Token::Type getBracket(char character);
    private:
        static const BracketMap s_BracketMap;
    };
}