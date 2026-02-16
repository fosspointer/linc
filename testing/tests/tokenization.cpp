#include "../Testing.hpp"
#include <linc/lexer/Lexer.hpp>

using namespace linc;

void stringAppendPrettyTokenization(std::string& result, std::string_view source, const std::vector<Token::Kind>& tokenization)
{
    std::string tokens;
    for(std::size_t i{0ul}; i < tokenization.size(); ++i)
    {
        if(i != 0ul)
            tokens.append(", ");
        tokens.append(Token::kindToString(tokenization[i]));
    }

    result.append(Logger::format("`$`: [$]\n", source, tokens));
}

DEFINE_TEST(NumberTest, "Testing that number literals are tokenized properly")
{
    const std::unordered_map<std::string_view, std::vector<Token::Kind>> tokenizations{
        {"0", std::vector<Token::Kind>{Token::Kind::LiteralNumber}},
        {"-1", std::vector<Token::Kind>{Token::Kind::LiteralNumber}},
        {"0xFF", std::vector<Token::Kind>{Token::Kind::LiteralNumber}},
        {"-0o777i32", std::vector<Token::Kind>{Token::Kind::LiteralNumber}},
        {"123_456.999_888", std::vector<Token::Kind>{Token::Kind::LiteralNumber}},
        {"-0x1f32.2f64", std::vector<Token::Kind>{Token::Kind::LiteralNumber, Token::Kind::LiteralNumber}},
        {"-0x1f32.abc", std::vector<Token::Kind>{Token::Kind::LiteralNumber, Token::Kind::Dot, Token::Kind::Identifier}}
    };

    std::string prettyString;

    for(const auto& tokenization: tokenizations)
        stringAppendPrettyTokenization(prettyString, tokenization.first, tokenization.second);

    TESTING_SNAPSHOT(prettyString, number_tokenizations);
}

TESTING_MAIN
