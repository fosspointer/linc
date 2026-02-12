#include <linc/lexer/Lexer.hpp>
#include <linc/lexer/Escape.hpp>
#include <linc/system/Logger.hpp>
#include <linc/lexer/Keywords.hpp>
#include <linc/system/Numbers.hpp>

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

    bool Lexer::match(char character)
    {
        if(peek() == character)
        {
            consume();
            return true;
        }
        else return false;
    }

    bool Lexer::match(std::string_view view)
    {
        if(view.contains(peek()))
        {
            consume();
            return true;
        }
        else return false;
    }

    Vector<Token> Lexer::operator()()
    {
        m_tokens.clear();
        ignoreShebang();
        while(m_characterIndex < m_sourceCode.size())
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
        else if(tokenizeLiteralCharacter()) return true;
        else if(tokenizeLiteralNumber()) return true;
        else if(tokenizeLiteralInterpolatedString()) return true;
        else return false;
    }

    bool Lexer::tokenizeLiteralString()
    {
        constexpr char string_literal_quote = '"';
        if(peek() != string_literal_quote)
            return false;

        auto [start, line] = getStartIndices();
        consume();

        char current = peek();
        while(current && current != string_literal_quote)
        {
            if(match('\\'))
            {
                match(string_literal_quote);
                current = peek();
                continue;
            }

            consume();
            current = peek();
        }

        if(!current || current != string_literal_quote)
            throw std::runtime_error("todo! unmatched double quotes in string");

        consume();
        m_tokens.push_back(Token(Token::Kind::LiteralString, viewFromBounds(start, m_characterIndex), m_file, line));

        return true;
    }

    bool Lexer::tokenizeLiteralNumber()
    {
        // For a number literal, the first character must be a digit OR a negation sign, followed by either a `.` or a digit, OR a `.` followed by a digit.
        //                                                  ^ e.g. 5, 42                     ^ e.g. -5, -.42                    ^ e.g. .1234
        if(!std::isdigit(peek()) 
        && (peek() != '-' || (peek(1ul) != '.' && !std::isdigit(peek(1ul))))
        && (peek() != '.' || !std::isdigit(peek(1ul))))
            return false;

        auto [start, line] = getStartIndices();

        bool has_digits_before_period = true;
        std::size_t digit_count = Numbers::defaultBase; // Specifies the range of our number system, e.g. 8 of octal, 16 for hexadecimal, etc...
        auto is_negative = match('-'); // Optionally match a negative sign
        if((has_digits_before_period = match('0'))) // Might specify base instead of digits
            digit_count = Numbers::getBaseByDescriptor(peek());

        std::string_view valid_digits = std::string_view{Numbers::digits, digit_count}; // Use the digit string up

        has_digits_before_period = match(valid_digits);
        char current = peek();
        while(valid_digits.contains(current) || (has_digits_before_period && current == '_'))
        {
            has_digits_before_period = true;
            consume();
            current = peek();
        }

        auto character_index_before_period = m_characterIndex;
        if(match('.'))
        {
            auto has_digits_after_period = match(valid_digits);
            current = peek();
            while(valid_digits.contains(current) || (has_digits_after_period && current == '_'))
            {
                has_digits_before_period = true;
                consume();
                current = peek();
            }

            // This is so as not to confuse a decimal point with the access operator (both are `.`)
            if(!has_digits_after_period)
            {
                if(!has_digits_before_period && is_negative)
                {
                    throw std::runtime_error("todo! expected number after `-.` character sequence");
                }

                m_characterIndex = character_index_before_period;
            }
        }
        m_tokens.push_back(Token(Token::Kind::LiteralNumber, viewFromBounds(start, m_characterIndex), m_file, line));
        return true;
    }

    bool Lexer::tokenizeLiteralCharacter()
    {
        constexpr char character_literal_quote = '\'';

        if(peek() != character_literal_quote)
            return false;

        auto [start, line] = getStartIndices();
        consume();

        if(match('\\'))
            consume();

        else if(match(character_literal_quote))
            throw std::runtime_error("todo! cannot have empty character literal");

        else
            consume();

        if(!match(character_literal_quote))
            throw std::runtime_error("todo! cannot have multi-character character literal");

        consume();
        m_tokens.push_back(Token(Token::Kind::LiteralCharacter, viewFromBounds(start, m_characterIndex), m_file, line));
        return true;
    }

    bool Lexer::tokenizeLiteralInterpolatedString()
    {
        return false;
    }

    bool Lexer::ignoreComments()
    {
        constexpr char comment_lead = '/';
        if(peek(1ul) != comment_lead || peek() != comment_lead)
            return false;

        char current = peek();
        while(current && current != '\n')
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
        if(peek() != '#' || peek(1ul) != '!')
            return;

        m_characterIndex += 2ul;

        char current = peek();
        while(current && current != '\n')
        {
            consume();
            current = peek();
        }
        consume();

        ++m_lineIndex;
    }

    bool Lexer::tokenizeWords()
    {
        if(!(std::isalpha(peek())) && peek() != '_')
            return false;

        auto [start, line] = getStartIndices();
        consume();

        auto current = peek();
        while(current && (std::isalnum(current) || current == '_'))
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
