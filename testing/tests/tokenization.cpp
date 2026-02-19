#include "../Testing.hpp"
#include <linc/lexer/Lexer.hpp>

using namespace linc;

void assertLexer(std::string_view source, const std::vector<Token::Kind>& tokenization)
{
    Lexer lexer(source, 0ul);
    auto got_tokenization = lexer();
    got_tokenization.pop_back(); // Pop EOF token

    std::size_t min_count = std::min(tokenization.size(), got_tokenization.size());
    for(std::size_t i{0ul}; i < min_count; ++i)
    {
        auto expected_kind = tokenization[i];
        auto got_kind = got_tokenization[i].kind;
        TESTING_ASSERT_MESSAGE(expected_kind == got_kind, Logger::format("Token #$ match failed. Expected '$', got '$'.", i, Token::kindToString(expected_kind), Token::kindToString(got_kind)));
    }

    TESTING_ASSERT_MESSAGE(tokenization.size() == got_tokenization.size(), Logger::format("Expected and actual token count do not match. Expected '$', got '$'.", tokenization.size(), got_tokenization.size()));
}

DEFINE_TEST(Numbers, "Testing that number literals are tokenized properly")
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

    for(const auto& tokenization: tokenizations)
        assertLexer(tokenization.first, tokenization.second);
}

DEFINE_TEST(Symbols, "Testing that symbol/operator sequences are tokenized properly")
{
    const std::unordered_map<std::string_view, std::vector<Token::Kind>> tokenizations{
        {"!", std::vector<Token::Kind>{Token::Kind::Bang}},
        {"!!", std::vector<Token::Kind>{Token::Kind::DoubleBang}},
        {"!!!", std::vector<Token::Kind>{Token::Kind::DoubleBang, Token::Kind::Bang}},
        {"!!!!", std::vector<Token::Kind>{Token::Kind::DoubleBang, Token::Kind::DoubleBang}},
        {"(>==!!)*//comment", std::vector<Token::Kind>{Token::Kind::ParenthesisLeft, Token::Kind::AngledRightEquals, Token::Kind::Equals, Token::Kind::DoubleBang, Token::Kind::ParenthesisRight, Token::Kind::Asterisk}},
    };

    for(const auto& tokenization: tokenizations)
        assertLexer(tokenization.first, tokenization.second);
}

TESTING_MAIN
