#include <linc/lexer/Lexer.hpp>
#include <linc/lexer/Escape.hpp>
#include <linc/system/Logger.hpp>
#include <linc/lexer/Keywords.hpp>

namespace linc
{
    std::string_view Lexer::viewFromBounds(std::size_t start, std::size_t end)
    {
        return std::string_view{m_sourceCode}.substr(start, end - start);
    }

    char Lexer::peek(std::size_t offset) const
    {
        if(auto target_index = m_characterIndex + offset; target_index < m_sourceCode.size())
            return m_sourceCode[target_index];
        else
            return '\0';
    }

    char Lexer::peek() const
    {
        if(m_characterIndex < m_sourceCode.size())
            return m_sourceCode[m_characterIndex];
        else
            return '\0';
    }

    Vector<Token> Lexer::operator()()
    {
        m_tokens.clear();
        ignoreShebang();
        while(peek())
        {
            if(ignoreSpace()) {}
            else if(ignoreComments()) {}
            else if(tokenizeLiterals()) {}
            else if(tokenizeWords()) {}
            else consume();
        }

        return m_tokens;
    }

    bool Lexer::ignoreSpace()
    {
        bool ignored{false};
        while(true)
        {
            switch(peek())
            {
            case ' ':
            case '\v':
            case '\f':
            case '\r':
                break;
            case '\n':
                ++m_lineIndex;
                break;
            default:
                return ignored;
            }
            ignored = true;
            consume();
        }
    }

    bool Lexer::tokenizeLiterals()
    {
        if(tokenizeLiteralString()) return true;
        else return false;
    }

    bool Lexer::tokenizeLiteralString()
    {
        constexpr char string_literal_quote = '"';
        if(peek() != string_literal_quote)
            return false;

        auto line = m_lineIndex;
        auto start = m_characterIndex;
        consume();

        char current = peek();
        while(current && line == m_lineIndex && current != string_literal_quote)
        {
            consume();
            current = peek();
        }

        if(!current || current != string_literal_quote)
            throw std::runtime_error("todo: this needs error handling");

        consume();
        m_tokens.push_back(Token(Token::Kind::LiteralString, viewFromBounds(start, m_characterIndex), m_file, line));

        return true;
    }

    bool Lexer::ignoreComments()
    {
        constexpr char comment_lead = '/';
        if(peek(1ul) != comment_lead || peek() != comment_lead)
            return false;

        char current;
        while(current = peek(), current && current != '\n')
        {
            consume();
            current = peek();
        }

        ++m_lineIndex;
        return true;
    }

    void Lexer::ignoreShebang()
    {
        ignoreSpace();
        if(peek() != '#' || peek() != '!')
            return;

        m_characterIndex = {};
        ++m_lineIndex;
    }

    bool Lexer::tokenizeWords()
    {
        if(!(std::isalpha(peek())) && peek() != '_')
            return false;

        auto line = m_lineIndex;
        auto start = m_characterIndex;
        consume();

        auto current = peek();
        while(current && line == m_lineIndex && (std::isalnum(current) || current == '_'))
        {
            consume();
            current = peek();
        }

        auto view = viewFromBounds(start, m_characterIndex);
        auto token_kind = Keywords::getKeywordOrIdentifier(view);
        m_tokens.push_back(Token(token_kind, view, m_file, line));
        return true;
    }
}
