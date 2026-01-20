#pragma once
#include <linc/system/Code.hpp>

namespace linc
{
    class Lexer final
    {
    public:
        Lexer(const Code::Source& source_code, std::size_t file)
            :m_sourceCode(source_code), m_file(file)
        {}

        Vector<Token> operator()();
    private:
        Token makeTokenFromValue(Token::Kind kind, std::size_t character_start, std::size_t character_end, std::size_t file, std::size_t line);
        bool ignoreSpace();
        bool ignoreComments();
        void ignoreShebang();
        bool tokenizeIdentifier();
        bool tokenizeLiterals();
        bool tokenizeLiteralString();
        // bool tokenizeLiteralCharacter();
        // bool tokenizeLiteralNumber();
        // bool tokenizeLiteralInterpolatedString();
        Code::Character consume() { return Code::consume(m_sourceCode, m_characterIndex, m_line); }
        [[nodiscard]] inline std::optional<Code::Character> peek(std::size_t offset = 0ul) { return Code::peek(m_sourceCode, m_characterIndex, m_line, 0ul); }

        const Code::Source m_sourceCode;
        std::size_t m_file, m_line{0ul}, m_characterIndex{0ul};
        Vector<Token> m_tokens;
    };
}

