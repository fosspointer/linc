#include <string>
#include <unordered_map>
#include <linc/lexer/Token.hpp>

#define LINC_KEYWORD_MAP_PAIR(first, second) std::pair<std::string, linc::Token::Type>(first, second)

namespace linc
{
    class Keywords final
    {
    public:
        using KeywordMap = std::unordered_map<std::string, Token::Type>;
        static Token::Type get(const std::string& keyword_string);
    private:
        const static KeywordMap s_KeywordMap;
    };
}