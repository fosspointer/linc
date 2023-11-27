#include <linc/lexer/Brackets.hpp>
#include <linc/system/Reporting.hpp>

#define LINC_BRACKET_MAP_PAIR(first, second) std::pair<char, linc::Token::Type>(first, second)
namespace linc
{
    
    const Brackets::BracketMap Brackets::s_BracketMap = {
        LINC_BRACKET_MAP_PAIR('(', Token::Type::ParenthesisLeft),
        LINC_BRACKET_MAP_PAIR(')', Token::Type::ParenthesisRight),
        LINC_BRACKET_MAP_PAIR('[', Token::Type::SquareLeft),
        LINC_BRACKET_MAP_PAIR(']', Token::Type::SquareRight),
        LINC_BRACKET_MAP_PAIR('{', Token::Type::BraceLeft),
        LINC_BRACKET_MAP_PAIR('}', Token::Type::BraceRight)
    };

    Token::Type Brackets::getBracket(char character)
    {
        auto find = s_BracketMap.find(character);
        if(find != s_BracketMap.end())
            return find->second;

        else return Token::Type::InvalidToken;
    }
}