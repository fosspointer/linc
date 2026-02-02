#include <linc/lexer/Keywords.hpp>

namespace linc
{
    const std::unordered_map<std::string_view, Token::Kind> Keywords::s_keywordMap{
        {"return", Token::Kind::KeywordReturn},
        {"fn", Token::Kind::KeywordFunction},
        {"if", Token::Kind::KeywordIf},
        {"else", Token::Kind::KeywordElse},
        {"while", Token::Kind::KeywordWhile},
        {"true", Token::Kind::KeywordTrue},
        {"false", Token::Kind::KeywordFalse},
        {"mut", Token::Kind::KeywordMutability},
        {"as", Token::Kind::KeywordAs},
        {"for", Token::Kind::KeywordFor},
        {"in", Token::Kind::KeywordIn},
        {"break", Token::Kind::KeywordBreak},
        {"continue", Token::Kind::KeywordContinue},
        {"record", Token::Kind::KeywordRecord},
        {"case", Token::Kind::KeywordCase},
        {"choice", Token::Kind::KeywordChoice},
        {"generic", Token::Kind::KeywordGeneric},
        {"alias", Token::Kind::KeywordAlias},
        {"scope", Token::Kind::KeywordNamespace},
        {"default", Token::Kind::KeywordDefault},
        {"finally", Token::Kind::KeywordFinally},
    };

    Token::Kind Keywords::getKeywordOrIdentifier(std::string_view word)
    {
        auto find = s_keywordMap.find(word);
        if(find != s_keywordMap.end())
            return find->second;
        else return Token::Kind::Identifier;
    }
}
