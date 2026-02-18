#pragma once
#include <cstdint>
#include <optional>
#include <string_view>

namespace linc
{
    struct Token final
    {
    public:
        enum class Kind : std::uint_least16_t
        {
            // Control Tokens
            EndOfFile, FormatStringStart, FormatStringEnd, FormatStringExpressionStart, FormatStringExpressionEnd,

            // Keywords 
            KeywordReturn, KeywordFunction, KeywordIf, KeywordElse, KeywordWhile, KeywordMutability, KeywordAs, KeywordFor, KeywordIn, KeywordBreak, KeywordContinue, KeywordRecord, KeywordCase, KeywordChoice, KeywordGeneric, KeywordAlias, KeywordNamespace, KeywordDefault, KeywordFinally,

            // Symbols
            Tilde, Bang, At, Pound, Percent, Caret, Ampersand, Asterisk, ParenthesisLeft, ParenthesisRight, Minus, Equals, Plus, SquareLeft, BraceLeft, SquareRight, BraceRight, Semicolon, Colon, Bar, Comma, AngledLeft, Dot, AngledRight, Slash, DoubleBang, DoublePound, DoubleAmpersand, DoubleMinus, DoubleEquals, DoublePlus, DoubleColon, DoubleBar, DoubleAngledLeft, DoubleDot, DoubleAngledRight, BangEquals, AsteriskEquals, MinusEquals, PlusEquals, ColonEquals, AngledLeftEquals, AngledRightEquals, SlashEquals,

            // Literals
            LiteralCharacter, LiteralString, LiteralNumber, LiteralTrue, LiteralFalse,

            // Identifiers
            Identifier
        };

        Token(Kind kind, std::string_view source_view, std::size_t file, std::size_t line)
            :kind(kind), file(file), line(line), lexeme(source_view)
        {}

        static std::string_view kindToString(Token::Kind kind);

        Kind kind;
        std::size_t file, line;
        std::string_view lexeme;
    };
}
