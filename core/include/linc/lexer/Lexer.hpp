#pragma once
#include <linc/system/Code.hpp>

namespace linc
{
    class Lexer final
    {
    public:
        Lexer(std::string_view source_code, std::size_t file)
            :m_sourceCode(source_code), m_file(file)
        {}

        Vector<Token> operator()();
    private:
        std::string_view viewFromBounds(std::size_t start, std::size_t end);
        bool ignoreSpace();
        bool ignoreComments();
        void ignoreShebang();
        bool tokenizeWords();
        bool tokenizeLiterals();
        bool tokenizeLiteralString();
        // bool tokenizeLiteralCharacter();
        // bool tokenizeLiteralNumber();
        // bool tokenizeLiteralInterpolatedString();

        inline void consume() { ++m_characterIndex; }
        [[nodiscard]] char peek(std::size_t offset) const;
        [[nodiscard]] char peek() const;

        std::string_view m_sourceCode;
        std::size_t m_file, m_lineIndex{0ul}, m_characterIndex{0ul};
        Vector<Token> m_tokens;
    };
}

