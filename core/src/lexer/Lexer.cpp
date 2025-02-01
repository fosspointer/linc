#include <linc/system/Reporting.hpp>
#include <linc/lexer/Brackets.hpp>
#include <linc/lexer/Keywords.hpp>
#include <linc/lexer/Operators.hpp>
#include <linc/lexer/Escape.hpp>
#include <linc/lexer/Lexer.hpp>
#include <linc/lexer/Token.hpp>

#define LINC_LEXER_STRING_LITERAL_QUOTE '"'
#define LINC_LEXER_PATH_LITERAL_QUOTE '`'
#define LINC_LEXER_CHARACTER_LITERAL_QUOTE '\''
#define LINC_LEXER_SLASH_SYMBOL '/'
#define LINC_LEXER_HEXADECIMAL_SPECIFIER "0x"

namespace linc
{
    Lexer::Lexer(const Code::Source& source_code, bool initialize_source)
        :m_sourceCode(source_code)
    {
        if(initialize_source)
            Reporting::setSource(m_sourceCode);
    }

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

        auto info = !m_sourceCode.empty()? Token::Info{.file = m_sourceCode.back().file, .line = m_sourceCode.back().line,
            .characterStart = m_sourceCode.back().text.size() - 1ul, .characterEnd = m_sourceCode.back().text.size() - 1ul}: Token::Info{};
        tokens.push_back(Token{.type = Token::Type::EndOfFile, .info = info});

        m_line = m_characterIndex = {};
        return tokens;
    }

    static std::optional<Token::NumberBase> getNumberBase(char c)
    {
        switch(c)
        {
        case 'd': return Token::NumberBase::Decimal;
        case 'x': return Token::NumberBase::Hexadecimal;
        case 'b': return Token::NumberBase::Binary;
        case 'o': return Token::NumberBase::Octal;
        default: return std::nullopt;
        }
    }

    bool Lexer::isSymbol(char c)
    {
        return std::string("!@#$%^&*-=+~`|<>:/.,;`").contains(c);
    }

    bool Lexer::digitHandle(char c, char next, size_t* decimal_count, Token::NumberBase base)
    {
        if(c == '.' && next != '.')
            return ++(*decimal_count);
        
        else return isDigit(c, base);
    }

    bool Lexer::digitPeek(char c, char next, Token::NumberBase base)
    {
        return isDigit(c, base) || (c == '.' && next != '.');
    }

    bool Lexer::hasDigit(std::string_view str, Token::NumberBase base)
    {
        for(const auto& c: str)
            if(isDigit(c, base))
                return true;
        
        return false;
    }

    bool Lexer::tokenizeSpace() const
    {
        if(m_line < m_sourceCode.size() && m_sourceCode[m_line].text.empty())
            return ++m_line;

        while(std::isspace(peek().value()))
        {
            consume();
            return true;
        }
        return false;
    }

    bool Lexer::tokenizeComments() const
    {
        if(peek(1ul) && peek(1ul).value() == LINC_LEXER_SLASH_SYMBOL && peek().value() == peek(1ul).value())
            return (m_characterIndex = {}, ++m_line, true);

        return false;
    }

    bool Lexer::tokenizeLiterals(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        if(tokenizeLiteralNumber(tokens, value_buffer));
        else if(tokenizeLiteralString(tokens, value_buffer));
        else if(tokenizeLiteralIncludePath(tokens, value_buffer));
        else if(tokenizeLiteralCharacter(tokens, value_buffer));
        else return false;
        
        return true;
    }

    bool Lexer::tokenizeLiteralNumber(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        auto character_index = m_characterIndex, line = m_line;
        std::string type_string;
        size_t decimal_count{};
        Token::NumberBase base{Token::NumberBase::Decimal};

        if(peek().has_value() && peek(1ul).has_value() && peek().value() == '0')
        {
            auto base_opt = getNumberBase(peek(1ul).value());
            if(base_opt) { consume(); consume(); base = base_opt.value(); }
        }

        if(digitHandle(peek().value(), peek(1ul)? peek(1ul).value(): '\0', &decimal_count, base) 
            || (peek().value() == '-' && peek(1ul).has_value() && digitPeek(peek(1ul).value(), peek(2ul)? peek(2ul).value(): '\0', base)))
        {
            Token::Info info = peek()? peek()->getInfo(): Token::Info{};

            do
            {
                value_buffer.push_back(consume());
            } while(peek() && peek()->getInfo().line == info.line && digitHandle(peek().value(), peek(1ul)? peek(1ul).value(): '\0', &decimal_count, base));

            while(peek() && peek()->getInfo().line == info.line && isalnum(peek().value()))
                type_string.push_back(consume());

            if(!value_buffer.empty() && value_buffer[0ul] == '.' && 
                (value_buffer.size() == 1ul || !isDigit(value_buffer[1ul], base)))

            {
                m_characterIndex = character_index;
                m_line = line;
                return false;
            }

            if(!hasDigit(value_buffer, base))
            {
                tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer, .info = info});
                return true;
            }

            info.characterEnd = info.characterStart + value_buffer.size();
            if(type_string.empty())
            {
                if(decimal_count == 0ul)
                    tokens.push_back(Token{.type = Token::Type::I32Literal, .value = value_buffer, .numberBase = base, .info = info});
                else if(decimal_count == 1ul && base == Token::NumberBase::Decimal)
                    tokens.push_back(Token{.type = Token::Type::F32Literal, .value = value_buffer, .numberBase = base, .info = info});
                else
                    tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer, .numberBase = base, .info = info});
                return true;
            }

            auto type = Types::kindFromUserStringSuffix(type_string);
            if(Types::isFloating(type) && base != Token::NumberBase::Decimal)
            {
                tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer, .numberBase = base, .info = info});
                return true;
            };

            switch(type)
            {
            case Types::Kind::i8: tokens.push_back(Token{.type = Token::Type::I8Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::i16: tokens.push_back(Token{.type = Token::Type::I16Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::i32: tokens.push_back(Token{.type = Token::Type::I32Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::i64: tokens.push_back(Token{.type = Token::Type::I64Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::u8: tokens.push_back(Token{.type = Token::Type::U8Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::u16: tokens.push_back(Token{.type = Token::Type::U16Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::u32: tokens.push_back(Token{.type = Token::Type::U32Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::u64: tokens.push_back(Token{.type = Token::Type::U64Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::f32: tokens.push_back(Token{.type = Token::Type::F32Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::f64: tokens.push_back(Token{.type = Token::Type::F64Literal, .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::_bool: tokens.push_back(Token{.type = Types::parseBoolean(value_buffer)? Token::Type::KeywordTrue: Token::Type::KeywordFalse,
                .value = value_buffer, .numberBase = base, .info = info}); break;
            case Types::Kind::_char: tokens.push_back(Token{.type = Token::Type::CharacterLiteral, .value = value_buffer, .numberBase = base, .info = info}); break;
            default: tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer + type_string, .numberBase = base, .info = info}); break;
            }
            return true;
        }
        else return false;
    }

    bool Lexer::tokenizeLiteralString(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        if(peek().value() == LINC_LEXER_STRING_LITERAL_QUOTE)
        {
            Token::Info info = consume().getInfo();
            while (peek()->getInfo().line == info.line && peek().has_value()
                && peek().value() != LINC_LEXER_STRING_LITERAL_QUOTE)
            {
                if(m_characterIndex + 1ul >= m_sourceCode[m_line].text.size())
                {
                    tokens.push_back(linc::Token{.type = linc::Token::Type::InvalidToken, .value = value_buffer, .info = info});
                    
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                        .span = TextSpan{.lineStart = m_line, .lineEnd = m_line, .spanStart = info.characterStart, .spanEnd = m_characterIndex, .file = info.file},
                        .message = Logger::format("$ Unmatched double-quote.", info)});
                    
                    return true;
                }
                else if(peek().value() == '\\' && peek(1ul).has_value())
                {
                    consume(); // Consume the '\' character (do not push)
                    value_buffer.push_back(Escape::get(consume()).value_or('\0')); // Push the escaped character
                }
                else value_buffer.push_back(consume());
            }

            consume(); // Consume ending quote
            tokens.push_back(linc::Token{.type = linc::Token::Type::StringLiteral, .value = value_buffer, .info = info});
            return true;
        }
        else return false;
    }

    bool Lexer::tokenizeLiteralIncludePath(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        if(peek().value() == LINC_LEXER_PATH_LITERAL_QUOTE)
        {
            auto start_index = m_characterIndex;
            Token::Info info = consume().getInfo();
            while (peek()->getInfo().line == info.line && peek().has_value()
                && peek().value() != LINC_LEXER_PATH_LITERAL_QUOTE)
            {
                if(m_characterIndex + 1ul >= m_sourceCode[m_line].text.size())
                {
                    tokens.push_back(linc::Token{.type = linc::Token::Type::InvalidToken, .value = value_buffer, .info = info});
                    
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                        .span = TextSpan{.lineStart = m_line, .lineEnd = m_line, .spanStart = info.characterStart - 1ul, .spanEnd = m_characterIndex, .file = info.file},
                        .message = Logger::format("$ Unmatched include-directory literal.", info)});
                    
                    return true;
                }
                else if(peek().value() == '\\' && peek(1ul).has_value())
                {
                    consume(); // Consume the '\' character (do not push)
                    value_buffer.push_back(Escape::get(consume()).value_or('\0')); // Push the quote
                }
                else value_buffer.push_back(consume());
            }

            std::string filepath{};
            for(const auto& include_directory: m_includeDirectories)
                if(auto path = include_directory + '/' + value_buffer; Files::exists(path))
                {
                    filepath = path;
                    break;
                }

            consume(); // Consume ending quote
            
            if(!filepath.empty())
                tokens.push_back(Token{.type = Token::Type::StringLiteral, .value = filepath, .info = info});
            else
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                    .span = TextSpan{.lineStart = m_line, .lineEnd = m_line, .spanStart = start_index, .spanEnd = m_characterIndex, .file = info.file},
                    .message = Logger::format("$ Include path does not exist.", info, value_buffer)
                });
                tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = value_buffer, .info = info});
            }
            return true;
        }
        else return false;
    }

    bool Lexer::tokenizeLiteralCharacter(std::vector<Token>& tokens, std::string& value_buffer) const
    {
        if(peek().value() == LINC_LEXER_CHARACTER_LITERAL_QUOTE)
        {
            Token::Info info = consume().getInfo();
            while(peek().has_value() && peek().value() != LINC_LEXER_CHARACTER_LITERAL_QUOTE)
            {
                if(m_characterIndex + 1ul >= m_sourceCode[m_line].text.size())
                {
                    tokens.push_back(linc::Token{.type = linc::Token::Type::InvalidToken, .value = value_buffer, .info = info});
                    
                    Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                        .span = TextSpan{.lineStart = m_line, .lineEnd = m_line, .spanStart = info.characterStart, .spanEnd = m_characterIndex, .file = info.file},
                        .message = Logger::format("$ Unmatched single-quote.", info)});
                    
                    return true;
                }
                else if(peek().value() == '\\' && peek(1ul).has_value())
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
                    .span = TextSpan{.lineStart = m_line, .lineEnd = m_line, .spanStart = m_characterIndex - 2ul, .spanEnd = m_characterIndex, .file = info.file},
                    .message = Logger::format("$ Character literal cannot be empty.", info)
                });
                tokens.push_back(Token{.type = Token::Type::CharacterLiteral, .value = "\0", .info = info});
                return true;
            }
            else if(value_buffer.size() > 1ul)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                    .span = TextSpan{.lineStart = m_line, .lineEnd = m_line, .spanStart = info.characterStart, .spanEnd = m_characterIndex, .file = info.file},
                    .message = Logger::format("$ More than one character in character literal.", info)
                });

            tokens.push_back(Token{.type = Token::Type::CharacterLiteral, .value = std::to_string(value_buffer[0ul]), .info = info});
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
            } while (peek() && peek()->getInfo().line == info.line && (std::isalnum(peek().value()) || peek().value() == '_'));

            info.characterEnd = info.characterStart + value_buffer.size();
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
        Token::Info info = peek()->getInfo();

        while(peek() && isSymbol(peek().value())) symbol.push_back(consume());
        info.characterEnd = info.characterStart + symbol.size();

        if(!symbol.empty())
        {
            auto token_type = Operators::getToken(symbol);
            
            if(token_type == Token::Type::InvalidToken)
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Error, .stage = Reporting::Stage::Lexer,
                    .span = TextSpan::fromTokenInfo(info),
                    .message = linc::Logger::format("$ Expected operator, found invalid character sequence.", info)});

            tokens.push_back(Token{.type = token_type, .value = token_type == Token::Type::InvalidToken? std::make_optional(symbol): std::nullopt, .info = info});
        }
        else 
        {
            tokens.push_back(Token{.type = Token::Type::InvalidToken, .value = symbol, .info = peek()->getInfo()});
            consume();
        }
    }
}
