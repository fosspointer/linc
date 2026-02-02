#pragma once
#include <linc/lexer/Token.hpp>
#include <unordered_map>

namespace linc
{
    class Keywords
    {
    public:
        Keywords() = delete;
        
        static Token::Kind getKeywordOrIdentifier(std::string_view word);
    private:
        const static std::unordered_map<std::string_view, Token::Kind> s_keywordMap;
    };
}
