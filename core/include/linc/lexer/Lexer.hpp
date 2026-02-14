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
        bool tokenizeLiteralCharacter();
        bool tokenizeLiteralNumber();
        bool tokenizeLiteralInterpolatedString();
        bool tokenizeSymbols();

        inline void consume() { ++m_characterIndex; }
        [[nodiscard]] char peek(std::size_t offset) const;
        [[nodiscard]] char peek() const;

        bool match(char character);
        bool match(std::string_view view);
        constexpr inline std::pair<std::size_t, std::size_t> getStartIndices() { return std::make_pair(m_characterIndex, m_lineIndex); }

        std::string_view m_sourceCode;
        std::size_t m_file, m_lineIndex{0ul}, m_characterIndex{0ul};
        Vector<Token> m_tokens;
    };
}

