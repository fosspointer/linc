#include <linc/lexer/Keywords.hpp>
#define LINC_KEYWORD_MAP_PAIR(first, second) std::pair<std::string, linc::Token::Type>(first, second)

namespace linc
{
    const Keywords::KeywordMap Keywords::s_KeywordMap = {
        LINC_KEYWORD_MAP_PAIR("fn", Token::Type::KeywordFunction),
        LINC_KEYWORD_MAP_PAIR("return", Token::Type::KeywordReturn),
        LINC_KEYWORD_MAP_PAIR("if", Token::Type::KeywordIf),
        LINC_KEYWORD_MAP_PAIR("elif", Token::Type::KeywordElseIf),
        LINC_KEYWORD_MAP_PAIR("else", Token::Type::KeywordElse),
        LINC_KEYWORD_MAP_PAIR("while", Token::Type::KeywordWhile),
        LINC_KEYWORD_MAP_PAIR("true", Token::Type::KeywordTrue),
        LINC_KEYWORD_MAP_PAIR("false", Token::Type::KeywordFalse),
    };

    Token::Type Keywords::get(const std::string& keyword_string)
    {
        auto find = s_KeywordMap.find(keyword_string);
        if(find != s_KeywordMap.end())
            return find->second;
        else return Token::Type::InvalidToken;
    }
}