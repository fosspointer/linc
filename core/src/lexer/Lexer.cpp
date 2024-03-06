#include <linc/system/Reporting.hpp>
#include <linc/lexer/Brackets.hpp>
#include <linc/lexer/Keywords.hpp>
#include <linc/lexer/Operators.hpp>
#include <linc/lexer/Escape.hpp>
#include <linc/lexer/Lexer.hpp>

#define LINC_LEXER_STRING_LITERAL_SYMBOL '"'
#define LINC_LEXER_CHARACTER_LITERAL_SYMBOL '\''

namespace linc
{
    Lexer::Lexer(const Code::Source& source_code)
        :m_sourceCode(source_code)
    {}

    auto Lexer::operator()() const -> std::vector<Token>
    {
        std::vector<Token> tokens;

        while(peek().has_value())
        {
            std::string value_buffer;

            // Try to tokenize different types of tokens, if one fails, move to the next one (fail returns false)
            if(tokenizeSpace());
            else if(tokenizeLiterals(tokens, value_buffer));
            else if(tokenizeWords(tokens, value_buffer));
            else if(tokenizeComments());
            else if(tokenizeBrackets(tokens, value_buffer)); 
            else tokenizeOperators(tokens, value_buffer);
        }

        tokens.push_back(Token{.type = Token::Type::EndOfFile, .info = Token::Info{.file = tokens.empty()? "": tokens.back().info.file, .line = m_lineIndex}});

        m_lineIndex = m_characterIndex = {};
        return tokens;
    }

    static bool isSymbol(char c)
    {
        // TODO: maybe do this differently in the future?
        return std::string("!@#$%^&*-=+~`|<>:/").contains(c);
    }

    static bool digitHandle(char c, size_t* decimal_count)
    {
        if(c == '.')
            return ++(*decimal_count);
        
        else return std::isdigit(c);
    }

    static bool digitPeek(char c)
    {
        return std::isdigit(c) || c == '.';
    }

    static bool hasDigit(std::string_view str)
    {
        bool digit{false};
        for(const auto& c: str)
            digit = std::isdigit(c)? true: digit;
        
        return digit;
    }

    bool Lexer::tokenizeSpace() const
    {
        if(m_lineIndex < m_sourceCode.size() && m_sourceCode[m_lineIndex].text.empty())
            return ++m_lineIndex;

        while(std::isspace(peek().value()))
        {
            consume();
            return true;
        }
        return false;
    }

    bool Lexer::tokenizeComments() const
    {
        if(peek().value() == '#')
        {
            m_characterIndex = {};
            ++m_lineIndex;
            return true;
        }
        return false;
    }

    bool Lexer::tokenizeLiterals(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        if(tokenizeLiteralNumber(tokens, value_buffer))
            return true;
        else if(tokenizeLiteralString(tokens, value_buffer))
            return true;
        else if(tokenizeLiteralCharacter(tokens, value_buffer))
            return true;
        return false;
    }

    bool Lexer::tokenizeLiteralNumber(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        std::string type_string;
        size_t decimal_count{};
        if(digitHandle(peek().value(), &decimal_count) 
            || (peek().value() == '-' && peek(1).has_value() && digitPeek(peek(1).value())))
        {
            Token::Info info{};

            do
            {
                info = peek()->getInfo();
                value_buffer.push_back(consume());
            } while(peek().has_value() && digitHandle(peek().value(), &decimal_count));

            while(peek().has_value() && isalnum(peek().value()))
            {
                info = peek()->getInfo();
                type_string.push_back(consume());
            }

            if(!hasDigit(value_buffer))
            {
                tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer, .info = info});
                return true;
            }

            if(type_string.empty())
            {
                if(decimal_count == 0)
                    tokens.push_back(Token{.type = Token::Type::I32Literal, .value = value_buffer, .info = info});
                else if(decimal_count == 1)
                    tokens.push_back(Token{.type = Token::Type::F32Literal, .value = value_buffer, .info = info});
                else
                    tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer, .info = info});
                return true;
            }

            auto type = Types::kindFromUserStringSuffix(type_string);

            switch(type)
            {
            case Types::Kind::i8: tokens.push_back(Token{.type = Token::Type::I8Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::i16: tokens.push_back(Token{.type = Token::Type::I16Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::i32: tokens.push_back(Token{.type = Token::Type::I32Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::i64: tokens.push_back(Token{.type = Token::Type::I64Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::u8: tokens.push_back(Token{.type = Token::Type::U8Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::u16: tokens.push_back(Token{.type = Token::Type::U16Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::u32: tokens.push_back(Token{.type = Token::Type::U32Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::u64: tokens.push_back(Token{.type = Token::Type::U64Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::f32: tokens.push_back(Token{.type = Token::Type::F32Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::f64: tokens.push_back(Token{.type = Token::Type::F64Literal, .value = value_buffer, .info = info}); break;
            case Types::Kind::_bool: tokens.push_back(Token{.type = Types::parseBoolean(value_buffer)? Token::Type::KeywordTrue: Token::Type::KeywordFalse,
                .value = value_buffer, .info = info}); break;
            case Types::Kind::_char: tokens.push_back(Token{.type = Token::Type::CharacterLiteral, .value = value_buffer, .info = info}); break;
            default: tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer + type_string, .info = info}); break;
            }
            return true;
        }
        else return false;
    }

    bool Lexer::tokenizeLiteralString(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        if(peek().value() == LINC_LEXER_STRING_LITERAL_SYMBOL)
        {
            Token::Info info = consume().getInfo();
            while (peek().has_value() && peek().value() != LINC_LEXER_STRING_LITERAL_SYMBOL)
            {
                if(m_characterIndex + 1ull >= m_sourceCode[m_lineIndex].text.size())
                {
                    tokens.push_back(linc::Token{.type = linc::Token::Type::InvalidToken, .value = value_buffer, .info = info});
                    
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                        .span = TextSpan{.text = m_sourceCode[m_lineIndex].text, .spanStart = m_characterIndex, .spanEnd = m_characterIndex + 1ull},
                        .message = "Unmatched double-quote."});
                    
                    return true;
                }
                else if(peek().value() == '\\' && peek(1ull).has_value())
                {
                    consume(); // Consume the '\' character (do not push)
                    value_buffer.push_back(Escape::get(consume()).value_or('\0')); // Push the quote
                }
                else value_buffer.push_back(consume());
            }

            consume(); // Consume ending quote
            tokens.push_back(linc::Token{.type = linc::Token::Type::StringLiteral, .value = value_buffer, .info = info});
            return true;
        }
        else return false;
    }

    bool Lexer::tokenizeLiteralCharacter(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        if(peek().value() == LINC_LEXER_CHARACTER_LITERAL_SYMBOL)
        {
            Token::Info info = consume().getInfo();
            while (peek().has_value() && peek().value() != LINC_LEXER_CHARACTER_LITERAL_SYMBOL)
            {
                if(m_characterIndex + 1ull >= m_sourceCode[m_lineIndex].text.size())
                {
                    tokens.push_back(linc::Token{.type = linc::Token::Type::InvalidToken, .value = value_buffer, .info = info});
                    
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                        .span = TextSpan{.text = m_sourceCode[m_lineIndex].text, .spanStart = m_characterIndex, .spanEnd = m_characterIndex + 1ull},
                        .message = "Unmatched single-quote."});
                    
                    return true;
                }
                else if(peek().value() == '\\' && peek(1ull).has_value())
                {
                    consume(); // Consume the '\' character (do not push)
                    value_buffer.push_back(Escape::get(consume()).value_or('\0')); // Push the quote
                }
                else value_buffer.push_back(consume());
            }

            consume(); // Consume ending quote
            
            if(value_buffer.empty())
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                    .span = TextSpan{.text = m_sourceCode[m_lineIndex].text, .spanStart = m_characterIndex, .spanEnd = m_characterIndex + 1ull},
                    .message = "Character literal cannot be empty."
                });
                tokens.push_back(Token{.type = Token::Type::CharacterLiteral, .value = "\0", .info = info});
                return true;
            }
            else if(value_buffer.size() > 1ull)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                    .span = TextSpan{.text = m_sourceCode[m_lineIndex].text, .spanStart = m_characterIndex, .spanEnd = m_characterIndex + 1ull},
                    .message = "More than one character in character literal."
                });

            tokens.push_back(Token{.type = Token::Type::CharacterLiteral, .value = std::string(1ull, value_buffer[0ull]), .info = info});
            return true;
        }
        else return false;
    }

    bool Lexer::tokenizeWords(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        if(std::isalpha(peek().value()) || peek().value() == '_')
        {
            auto info = peek()->getInfo();

            do
            {
                value_buffer.push_back(consume());
            } while (peek().has_value() && (std::isalnum(peek().value()) || peek().value() == '_'));

            auto token_type = Keywords::get(value_buffer);
            
            if(token_type == Token::Type::KeywordTrue || token_type == Token::Type::KeywordFalse)
                tokens.push_back(Token{.type = token_type, .value = value_buffer, .info = info});
            else if(token_type != Token::Type::InvalidToken)
                tokens.push_back(Token{.type = token_type, .info = info});
            else
                tokens.push_back(Token{.type = Token::Type::Identifier, .value = value_buffer, .info = info});
            return true;
        }
        return false;
    }

    bool Lexer::tokenizeBrackets(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        auto bracket = Brackets::getBracket(peek().value());

        if(bracket != Token::Type::InvalidToken)
        {
            tokens.push_back(Token{.type = bracket, .info = peek()->getInfo()});
            consume();
            return true;
        }
        return false;
    }

    void Lexer::tokenizeOperators(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        std::string symbol;
        Token::Info info;

        while (peek().has_value() && isSymbol(peek().value()))
        {
            info = peek()->getInfo();
            symbol.push_back(consume());
        }

        if(!symbol.empty())
            tokens.push_back(Token{.type = Operators::get(symbol), .info = info});
        else 
        {
            tokens.push_back(Token{.type = Token::Type::InvalidToken, .info = peek()->getInfo()});
            consume();
        }
    }
}