#pragma once
#include <linc/lexer/Token.hpp>
#include <linc/Include.hpp>

namespace linc
{
    /// @brief Lexer-token representation of bracket-pairs. 
    class Brackets final
    {
    public:
        using BracketMap = std::vector<std::pair<char, Token::Type>>; 
        static Token::Type getBracket(char character);
        static char getChar(Token::Type token_type);
    private:
        static const BracketMap s_BracketMap;
    };
}